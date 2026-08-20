#pragma once
#include "ast/ast.hpp"
#include "lexer/lexer.hpp"
#include <string>
#include <vector>

class Parser {
    public:
        explicit Parser(std::vector<Token> tokens);
        ProgramNode parse();
    
    private:
        std::vector<Token> tokens_;
        size_t pos = 0;

        DeclarationNode parseDeclaration();
        PacketNode parsePacketDeclaration();
        EnumNode parseEnumDeclaration();
        FieldNode parseFieldDeclaration();
        Endian parseEndianAnnotation();
        ChecksumAlgo parseChecksumAnnotation();
        EnumValueNode parseEnumValue();
        bool peekIsBitAnnotation();
        bool peekIsAutoAnnotation();

        const Token& peek() const;
        const Token& advance();
        bool check(TokenType type) const;
        bool match(TokenType type);
        const Token& expect(TokenType type, const std::string& errorMessage);
        bool isAtEnd() const;

};