#pragma once
#include "ast/ast.hpp"
#include <string>

class CodeGenerator {
public:
    std::string generate(const ProgramNode& program);
private:
    std::string generateEnum(const EnumNode& node);
    std::string generatePacket(const PacketNode& node);
    std::string generatePackFunction(const PacketNode& node);
    std::string generateUnPackFunction(const PacketNode& node);
    std::string primitiveTypeToCpp(PrimitiveType type) const;
}