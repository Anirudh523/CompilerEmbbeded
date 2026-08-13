#include "parser.hpp"
#include "/Users/anirudh/Documents/GitHub/CompilerEmbbeded/dsl-compiler/src/lexer/token.hpp"


bool isPrimitiveType(TokenType type) {
    switch(type) {
        case TokenType::UInt8:
        case TokenType::UInt16:
        case TokenType::UInt32:
        case TokenType::Int8:
        case TokenType::Int16:
        case TokenType::Int32:
        case TokenType::Float32:
        case TokenType::Crc16:
            return true;
        default:
            return false;
    }
}

PrimitiveType tokentoPrimitive(TokenType type) {
    switch(type) {
        case TokenType::UInt8: return PrimitiveType::UInt8;
        case TokenType::UInt16: return PrimitiveType::UInt16;
        case TokenType::UInt32: return PrimitiveType::UInt32;
        case TokenType::Int8: return PrimitiveType::Int8;
        case TokenType::Int16: return PrimitiveType::Int16;
        case TokenType::Int32: return PrimitiveType::Int32;
        case TokenType::Float32: return PrimitiveType::Float32;
        case TokenType::Crc16: return PrimitiveType::Crc16;
        default:
            throw std::runtime_error("Token is not a primitive type");
    }
}


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

bool Parser::peakisBitAnnotation() {
    if(check(TokenType::At)) {
        Token nextToken = tokens_[pos + 1];
        if(nextToken.type == TokenType::Identifier && std::string(nextToken.lexeme) == "bit") {
            return true;
        }
    }
    return false;
}

bool Parser::peakisAutoAnnotation() {
    if(check(TokenType::At)) {
        Token nextToken = tokens_[pos + 1];
        if(nextToken.type == TokenType::Identifier && std::string(nextToken.lexeme) == "auto") {
            return true;
        }
    }
    return false;
}


PacketNode Parser::parseFieldDeclaration(){
    FieldNode field;
    if(isPrimitiveType(peek().type)){
        field.primitiveType = tokentoPrimitive(advance().type);
    } else {
        Token typeToken = expect(TokenType::Identifier, "Expected a variable type name here");
        std::string type_name = std::string(typeToken.lexeme);
        field.typeRef = type_name;
    }

    Token nameToken = expect(TokenType::Identifier, "Expect a variable name here");
    field.name = std::string(nameToken.lexeme);

    if(match(TokenType::At)){
        if(peekIsBitAnnotation()){
            advance();
            advance();
            expect(TokenType::LParen, "Expected '(' after the @bit annotation.");
            Token bitWidthToken = expect(TokenType::IntegerLiteral, "Expected a bit width value here");
            field.bitWidth = std::stoi(std::string(bitWidthToken.lexeme));
            expect(TokenType::RParen, "Expected ')' after the bit width value.");
        } else if(peekIsAutoAnnotation()){
            advance();                                            
            expect(TokenType::Identifier, "expected 'auto'");      
            field.isAuto = true;
            if (match(TokenType::LParen)) {
                field.checksumAlgo = parseChecksumAlgo();
                expect(TokenType::RParen, "expected ')'");
            }
        } else {
            throw std::runtime_error("Expected a valid annotation after '@' at line: " + std::to_string(peek().line));
        }
    }



        
    
}