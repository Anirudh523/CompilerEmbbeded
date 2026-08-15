#pragma once
#include <string>
#include <string_view>
enum class TokenType {
    // Keywords
    Packet, Enum,
    // Types
    UInt8, UInt16, UInt32, Int8, Int16, Int32, Float32, Crc16,
    // Punctuation
    LBrace, RBrace, LBracket, RBracket, LParen, RParen,
    Semicolon, Comma, Colon, Equals, At,
    // Literals
    Identifier, IntegerLiteral,
    // Annotations (or just lex '@' + identifier and let the parser interpret)
    // End
    Eof,
    Invalid
};
struct Token {
    TokenType type;
    std::string_view lexeme;   // the raw text, e.g. "packet", "ImuFrame", "16"
    int line;
    int column;
};