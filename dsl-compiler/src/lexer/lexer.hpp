#include <string>
#include "token.hpp"
#include <vector>
#include <iostream>
#include <cctype>
#pragma once

class Lexer {
    public:
        Lexer(const std::string& input);

        Token scanToken();

        Token identifierOrKeyword();

        Token number();

        void skipWhitespaceAndComments();

        std::vector<Token> tokenize();

    private:
        int col_;
        int pos_;
        int line_;
        std::string source_;
        char advance();
        char peek() const;
        char peekNext() const;
        bool isAtEnd() const;
        Token makeToken(const TokenType& type);
};