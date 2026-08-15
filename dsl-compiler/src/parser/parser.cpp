#include "parser.hpp"


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
    if (!isAtEnd()) pos++;
    return tokens_[pos - 1];
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

bool Parser::peekIsBitAnnotation() {
    Token nextToken = tokens_[pos];
    if(nextToken.type == TokenType::Identifier && std::string(nextToken.lexeme) == "bits") {
        return true;
    }
    
    return false;
}

bool Parser::peekIsAutoAnnotation() {
    Token nextToken = tokens_[pos];
    if(nextToken.type == TokenType::Identifier && std::string(nextToken.lexeme) == "auto") {
        return true;
    }
    return false;
}


ChecksumAlgo Parser::parseChecksumAnnotation() {
    Token algoToken = expect(TokenType::Identifier, "Expected a checksum algorithm name here");
    std::string algoName = std::string(algoToken.lexeme);
    if(algoName == "crc16") {
        return ChecksumAlgo::Crc16;
    } else if(algoName == "crc32") {
        return ChecksumAlgo::Crc32;
    } else if(algoName == "sum8") {
        return ChecksumAlgo::Sum8;
    } else if(algoName == "xor8") {
        return ChecksumAlgo::Xor8;
    } else {
        throw std::runtime_error("Unknown checksum algorithm: " + algoName + " at line: " + std::to_string(algoToken.line));
    }  

}

FieldNode Parser::parseFieldDeclaration(){
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

    if(match(TokenType::LBracket)){
        Token sizeToken = expect(TokenType::IntegerLiteral, "Expected an array size value here.");
        field.arraySize = std::stoi(std::string(sizeToken.lexeme));
        expect(TokenType::RBracket, "Expected a closing bracket ']' after the array size.");
    }
    if(match(TokenType::At)){
        if(peekIsBitAnnotation()){
            advance();
            expect(TokenType::LParen, "Expected '(' after the @bit annotation.");
            Token bitWidthToken = expect(TokenType::IntegerLiteral, "Expected a bit width value here");
            field.bitWidth = std::stoi(std::string(bitWidthToken.lexeme));
            expect(TokenType::RParen, "Expected ')' after the bit width value.");
        } else if(peekIsAutoAnnotation()){
            advance();
            field.isAuto = true;
            if (match(TokenType::LParen)) {
                field.checksum = parseChecksumAnnotation();
                expect(TokenType::RParen, "expected ')'");
            }
        } else {
            throw std::runtime_error("Expected a valid annotation after '@' at line: " + std::to_string(peek().line));
        }
    }
    expect(TokenType::Semicolon, "Expected a semicolon after the field declaration.");
    return field;    
}

Endian Parser::parseEndianAnnotation() {
    Token key = expect(TokenType::Identifier, "Expected 'endian' after '@'");
    if(std::string(key.lexeme) != "endian") {
        throw std::runtime_error("Expected 'endian' after '@' at line: "+ std::to_string(key.line));
    }
    expect(TokenType::LParen, "Expected '(' after '@endian'");
    Token endianToken = expect(TokenType::Identifier, "Need to specify an endian type after @endian");
    std::string endianType = std::string(endianToken.lexeme);

    Endian result;
    if(endianType == "big") {
        result = Endian::Big;
    } else if(endianType == "little"){
        result = Endian::Little;
    } else {
        throw std::runtime_error("Unknown endian type: " + endianType + " at line: " + std::to_string(endianToken.line));
    }
    expect(TokenType::RParen, "Expected ')' after endian type");
    return result;
}

EnumNode Parser::parseEnumDeclaration() {
    EnumNode enumNode;
    expect(TokenType::Enum, "Expected a enum keyword here.");
    Token nameToken = expect(TokenType::Identifier, "Expected an enum name here.");
    enumNode.name = std::string(nameToken.lexeme);

    if(match(TokenType::Colon)) {
        if(!isPrimitiveType(peek().type)) {
            throw std::runtime_error("Backing type for enum must be a primitive type at line: " + std::to_string(peek().line));
        }
        enumNode.backingType = tokentoPrimitive(advance().type);
    }

    expect(TokenType::LBrace, "Expect an opening brace { here");

    enumNode.values.push_back(parseEnumValue());
    while(match(TokenType::Comma)) {
        enumNode.values.push_back(parseEnumValue());
    }

    expect(TokenType::RBrace, "Expected a closing brace } after the enum declaration.");
    return enumNode;
}

EnumValueNode Parser::parseEnumValue() {
    EnumValueNode value;
    Token valueNameToken = expect(TokenType::Identifier, "Expected an enum value name here.");
    value.name = std::string(valueNameToken.lexeme);
    if(match(TokenType::Equals)){
        Token valueToken = expect(TokenType::IntegerLiteral, "Expected an integer value for the enum value.");
        value.value = std::stoi(std::string(valueToken.lexeme));
    } else {
        value.value = 0;
    }
    return value;
}

ProgramNode Parser::parse() {
    ProgramNode program;
    while(!isAtEnd()) {
        program.declarations.push_back(parseDeclaration());
    }
    return program;
}