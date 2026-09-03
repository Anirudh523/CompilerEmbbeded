# dsl-compiler

A small compiler that turns a domain-specific packet-definition language into
zero-copy, embedded-safe C++ structs — with automatic byte layout, bitfield
packing, and CRC16 checksum generation. Built for STM32/Cortex-M4 targets
(and any little-endian embedded platform), targeting `-fno-exceptions
-fno-rtti` compatibility.

Given a `.pkt` file describing a wire-format message:

```
packet ImuFrame {
    uint16 seq;
    float32 accel_x;
    float32 accel_y;
    float32 accel_z;
    uint8   flags @bits(4);
    crc16   checksum @auto;
}
```

`dslc` generates a real C++ header with a matching struct, a `pack()`
function to serialize it to a byte buffer, and an `unpack()` function to
deserialize (and CRC-verify) it back — so two boards using code generated
from the same `.pkt` file are guaranteed to agree on the exact byte layout,
with no hand-written, hand-synchronized structs on either side.

## Why this exists

Hand-written packet structs on two different microcontrollers drift out of
sync easily — a field added on one side and forgotten on the other produces
silent, hard-to-debug corruption. This compiler makes the `.pkt` file the
single source of truth: one file, one compile step, matching generated code
on every device that includes it.

## Pipeline

```
source (.pkt file)
   │
   ▼
[ Lexer ]        hand-written, produces a flat token stream
   │
   ▼
[ Parser ]       recursive descent, builds a std::variant-based AST
   │
   ▼
[ Semantic       resolves type references (including forward references),
  Analysis ]     detects circular packet nesting, validates bitfield widths,
   │             computes byte offsets/bit positions, resolves enum backing
   │             types
   ▼
[ Optimizer ]    reorders fields to minimize padding (skipped for packets
   │             using bitfields or checksums, where field order is
   │             semantically load-bearing)
   ▼
[ Codegen ]      emits a C++ struct, static_assert, pack(), and unpack()
   │
   ▼
generated_output.hpp
```

Each stage is a separate class (`Lexer`, `Parser`, `SemanticAnalyzer`,
`Optimizer`, `CodeGenerator`) with a narrow, single-purpose interface, so
each can be tested and reasoned about independently.

## Grammar

Full grammar: [`grammar/grammar.ebnf`](grammar/grammar.ebnf). Two top-level
declarations only — `packet` and `enum` — deliberately no functions, no
expressions, no control flow. The language describes the *shape* of data:
types, layout, byte order, and bit-packing. It does not describe behavior.

### Design constraints (permanent)

- No pointers, no dynamic-size types, no heap allocation, no unions (yet).
  Every packet has a statically computable size.
- No user-defined functions or arbitrary expressions. The only computed
  behavior is a small, fixed set of compiler-known annotations.

### Supported types

`uint8` `uint16` `uint32` `int8` `int16` `int32` `float32` `crc16`, plus
references to other declared `packet`/`enum` types (nesting).

### Annotations

| Annotation | Meaning |
|---|---|
| `@bits(n)` | Packs the field into `n` bits, shared with adjacent bitfields in the same byte |
| `@auto` / `@auto(algo)` | Computed field — codegen fills it in. Bare `@auto` implies CRC16 for `crc16`-typed fields; other types require an explicit algorithm |
| `@endian(big\|little)` | Packet-level byte order (see Known Limitations) |

Enums support an optional explicit backing type (`enum Mode : uint8 { ... }`);
if omitted, semantic analysis picks the smallest signed/unsigned type that
fits every declared value.

## Building

```bash
cmake -S . -B build
cmake --build build
```

Requires a C++20 compiler (`-fno-exceptions`/`-fno-rtti` for the *generated*
output only — the compiler itself uses normal C++ exceptions for error
reporting).

## Usage

```bash
./build/dslc path/to/file.pkt
```

Parses, validates, optimizes, and generates `generated_output.hpp` in the
current directory. Errors (unknown types, circular references, malformed
syntax, bitfield overflow) are reported with line numbers and a clear
message, and exit with a non-zero status.

## Example output

For the `ImuFrame` packet above, `dslc` produces:

```cpp
struct ImuFrame {
    uint16_t seq;
    float accel_x;
    float accel_y;
    float accel_z;
    uint8_t flags;
    uint16_t checksum;
};
static_assert(sizeof(ImuFrame) == 17);

inline void pack(const ImuFrame& value, uint8_t* buffer) {
    std::memcpy(buffer + 0, &value.seq, 2);
    std::memcpy(buffer + 2, &value.accel_x, 4);
    std::memcpy(buffer + 6, &value.accel_y, 4);
    std::memcpy(buffer + 10, &value.accel_z, 4);
    std::memcpy(buffer + 14, &value.flags, 1);
    uint16_t computed_crc = crc16(buffer, 15);
    std::memcpy(buffer + 15, &computed_crc, 2);
}

inline bool unpack(const uint8_t* buffer, ImuFrame& value) {
    std::memcpy(&value.seq, buffer + 0, 2);
    std::memcpy(&value.accel_x, buffer + 2, 4);
    std::memcpy(&value.accel_y, buffer + 6, 4);
    std::memcpy(&value.accel_z, buffer + 10, 4);
    std::memcpy(&value.flags, buffer + 14, 1);
    uint16_t received_crc;
    std::memcpy(&received_crc, buffer + 15, 2);
    uint16_t computed_crc = crc16(buffer, 15);
    if (received_crc != computed_crc) return false;
    return true;
}
```

`unpack()` returns `bool` — `false` means the CRC check failed and the data
should be discarded, not trusted.

For a packet using `@bits`, fields sharing a byte are packed with shift/mask
logic instead of `memcpy`, and the `static_assert` on `sizeof(...)` is
omitted for that struct (the in-memory C++ struct layout and the packed
wire-format size are intentionally different — see below).

## Design decisions worth knowing

- **CRC16 variant**: CRC-16/CCITT-FALSE (polynomial `0x1021`, initial value
  `0xFFFF`). Both sides of a communication link must use code generated
  from the same compiler for CRCs to match — this isn't a general-purpose
  CRC library, it's a fixed, internally-consistent choice.
- **Bitfields**: packed MSB-first within a shared byte, computed and
  generated manually (shift + mask + OR) rather than using real C++
  bitfield syntax (`: 4`), since C++ bitfield packing order is
  implementation-defined across compilers — this compiler's approach is
  fully deterministic instead.
- **Generated struct size vs. wire size**: for non-bitfield packets, the
  C++ struct's `sizeof` and the packed wire size are identical, and this is
  checked with a `static_assert`. For packets using `@bits`, they're
  different by design (each bitfield still gets its own full C++ struct
  member for ergonomic access, but multiple bitfields share one byte on the
  wire) — no `static_assert` is emitted for those packets.
- **Checksum verification asymmetry**: `pack()` *computes* the checksum;
  `unpack()` *verifies* it against what was received, rather than blindly
  copying it — this is the actual point of having a checksum.

## Known limitations

- **`@endian(big)` is parsed and validated but not yet implemented in
  codegen.** Packets that declare it are rejected at compile time with a
  clear error, rather than silently generating incorrect little-endian
  output. Both current target devices are natively little-endian, so this
  hasn't blocked real use; big-endian byte-swapping is a planned addition.
- **No framing or acknowledgment layer.** CRC16 detects bit-level
  corruption *within a received packet* — it does not detect partial or
  lost transmissions, and there is no retransmission mechanism. A real
  deployment would layer sync markers and/or ACK/NACK on top of the
  structs this compiler generates.
- **Only `sum8`, `xor8`, `crc32`, and `crc16` checksum algorithms are
  declarable in the grammar** (`@auto(algo)`); currently only `crc16` is
  implemented in codegen.
- **No unions, optional fields, conditional fields, or computed array
  lengths** — deliberately deferred; see `grammar/grammar.ebnf` section 8
  for the full list of explicitly out-of-scope features and why.

## Project structure

```
dsl-compiler/
├── grammar/grammar.ebnf       # formal language spec
├── src/
│   ├── lexer/                 # text → tokens
│   ├── parser/                # tokens → AST (recursive descent)
│   ├── ast/                   # AST node definitions (std::variant-based)
│   ├── sema/                  # symbol table, type resolution, layout
│   ├── optimize/               # field-reordering pass
│   ├── codegen/                # AST → C++ source text
│   └── main.cpp
├── examples/                  # sample .pkt files exercising each grammar feature
├── tests/unit/                # Catch2 tests per stage
└── CMakeLists.txt
```

## Status

Full pipeline (lexer → parser → semantic analysis → optimizer → codegen)
is implemented and verified end-to-end, including CRC16 checksum
generation/verification and byte-level bitfield packing, confirmed via
round-trip tests (pack → corrupt → unpack correctly rejects; pack → unpack
correctly round-trips). Hardware validation on an actual STM32 target is in
progress.
