#pragma once
#include <string>
#include <vector>
#include <variant>
#include <optional>

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

enum class Endian { Little, Big };

enum class ChecksumAlgo { Crc16, Crc32, Sum8, Xor8 };

struct FieldNode {
    std::string name;
    std::optional<PrimitiveType> primitiveType;
    std::optional<std::string> typeRef;

    std::optional<int> arraySize;
    std::optional<int> bitWidth;
    bool isAuto = false;
    std::optional<ChecksumAlgo> checksum;

    int byteOffset = -1;
    int byteSize = -1;
};

struct PacketNode {
    std::string name;
    std::vector<FieldNode> fields;
    std::optional<Endian> endian;

    int totalSize = -1;
};

struct EnumValueNode {
    std::string name;
    int value;
};

struct EnumNode {
    std::string name;
    std::optional<PrimitiveType> backingType;
    std::vector<EnumValueNode> values;
};

using DeclarationNode = std::variant<PacketNode, EnumNode>;

struct ProgramNode {
    std::vector<DeclarationNode> declarations;
};