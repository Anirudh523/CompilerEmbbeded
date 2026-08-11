#pragma once
#include <string>
#include "token.hpp"
#include <vector>
#include <cctype>

class Lexer {
    public:
        explicit Lexer(const std::string& input);
        std::vector<Token> tokenize();

    private:
        std::string source_;
        int pos_ = 0;
        int line_ = 1;
        int col_ = 1;

        Token scanToken();
        Token identifierOrKeyword();
        Token number();
        void skipWhitespaceAndComments();
        Token makeToken(const TokenType& type);

        char advance();
        char peek() const;
        char peekNext() const;
        bool isAtEnd() const;
};