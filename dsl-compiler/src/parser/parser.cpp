#include "parser.hpp"
#include "/Users/anirudh/Documents/GitHub/CompilerEmbbeded/dsl-compiler/src/lexer/token.hpp"
Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)), pos(0) {}

const Token& Parser::peek() const {
    return tokens_[pos];
}

const Token& Parser::advance() {
    return tokens_[pos++];
}

bool Parser::check(TokenType type) const {
    return !isAtEnd() && peek().type == type;
}

bool Parser::match(TokenType type) {
    if(check(type)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::expect(TokenType type, const std::string& errorMessage) {
    if(check(type)) {
        return advance();
    }
    throw std::runtime_error("Parse error at line: " + std::to_string(peek().line) + ": " + errorMessage);
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::Eof;
}

DeclarationNode Parser::parseDeclaration() {
    if(check(TokenType::Packet)) {
        return parsePacketDeclaration();
    } else if(check(TokenType::Enum)){
        return parseEnumDeclaration();
    } else {
        throw std::runtime_error("Expected 'packet' or 'enum' at line: " + std::to_string(peek().line));
    }
}

PacketNode Parser::parsePacketDeclaration() {
    expect(TokenType::Packet, "Expected a packet keyword here.");
    Token nameToken = expect(TokenType::Identifier, "Expected a packet name here.");

    PacketNode packet;
    packet.name = std::string(nameToken.lexeme);

    if(match(TokenType::At)) {
        packet.endian = parseEndianAnnotation();
    }
    expect(TokenType::LBrace, "Expected a opening brace '{' after packet name.");
    while(!isAtEnd() && !check(TokenType::RBrace)) {
        packet.fields.push_back(parseFieldDeclaration());
    }
    expect(TokenType::RBrace, "Expected a closing brace '}' after packet fields.");
    return packet;
}

PacketNode Parser::parseFieldDeclaration(){
    FieldNode field;
    
}