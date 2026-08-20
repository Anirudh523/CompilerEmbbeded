#pragma once
#include "ast/ast.hpp"
#include <unordered_map>
#include <string>
#include <stdexcept>
using namespace std;

class SemanticAnalyzer {
    public:
        void analyze(ProgramNode& program);
    private:
        std::unordered_map<std::string, PacketNode*> packetTable_;
        std::unordered_map<std::string, EnumNode*> enumTable_;

        void buildSymbolTable(ProgramNode& program);
        void resolvePacket(PacketNode& packet);
        void resolveField(FieldNode& field, const std::string& packetName);
        void validateBitfields(PacketNode& packet);
        void computeLayout(PacketNode& packet);
        void resolveEnumBackingType(EnumNode& node);

        int primitiveSizeInBytes(PrimitiveType type) const;
};