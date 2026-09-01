#pragma once
#include <string_view>

enum class TokenType {
    Packet, Enum,
    UInt8, UInt16, UInt32, Int8, Int16, Int32, Float32, Crc16,
    LBrace, RBrace, LBracket, RBracket, LParen, RParen, Semicolon, Comma,
    Colon, Equals, At,
    Identifier, IntegerLiteral,
    Eof, Invalid
};

struct Token {
    TokenType type;
    std::string_view lexeme;
    int line;
    int column;
};
