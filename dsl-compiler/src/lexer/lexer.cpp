#include "lexer.hpp"
#include "token.hpp"
#pragma once
using namespace std;

Lexer::Lexer(const std::string& input) : source_(input), pos_(0), line_(1), col_(1) {}

Token Lexer::scanToken() {
    skipWhitespaceAndComments();
    if (isAtEnd()) return makeToken(TokenType::Eof);
    char c = advance();
    switch(c) {
        case '{': return makeToken(TokenType::LBrace);
        case '}': return makeToken(TokenType::RBrace);
        case '[': return makeToken(TokenType::LBracket);
        case ']': return makeToken(TokenType::RBracket);
        case '(': return makeToken(TokenType::LParen);
        case ')': return makeToken(TokenType::RParen);
        case ';': return makeToken(TokenType::Semicolon);
        case ',': return makeToken(TokenType::Comma);
        case ':': return makeToken(TokenType::Colon);
        case '=': return makeToken(TokenType::Equals);
        case '@': return makeToken(TokenType::At);
        default:
            if(isalpha(c) || c == '_') {
                return identifierOrKeyword();
            } else if(isdigit(c)) {
                return number();
            } else {    
            return makeToken(TokenType::Invalid);
            }

    }

}

Token Lexer::makeToken(const TokenType& type) {
    int start = pos_ - 1;
    int line = line_;
    int column = col_ - 1;
    int length = 1;
    return Token{type, std::string_view(source_).substr(start, length), line, column};
}

char Lexer::advance() {
    if(isAtEnd()) return '\0';
    char c = source_[pos_++];
    if(c == '\n') {
        line_++;
        col_ = 1;
    } else {
        col_++;
    }
    return c;
}

