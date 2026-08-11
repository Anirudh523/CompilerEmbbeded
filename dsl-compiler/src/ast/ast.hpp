#pragma once
#include <string>
#include <vector>
#include <variant>
#include <optional>
using namespace std;

enum class PrimitiveType {
    UInt8,
    UInt16,
    UInt32,
    Int8,
    Int16,
    Int32,
    Float32,
    Crc16
};

enum class Endian { Little, Big};

enum class ChecksumAlgo {Crc16, Crc32, Sum8, Xor8};

struct FieldNode {
    std::string name;
    std::optional<PrimitiveType> primitiveType;

}