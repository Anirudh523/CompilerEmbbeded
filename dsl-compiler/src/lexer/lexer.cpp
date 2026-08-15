#include "lexer.hpp"
#include "token.hpp"
using namespace std;

Lexer::Lexer(const std::string& input) : source_(input), pos_(0), line_(1), col_(1) {}

Token Lexer::scanToken() {
    skipWhitespaceAndComments();
    if (isAtEnd()) return Token{TokenType::Eof, std::string_view{}, line_, col_};
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

char Lexer::peek() const {
    if(isAtEnd()) return '\0';
    return source_[pos_];
}

bool Lexer::isAtEnd() const {
    return pos_ >= source_.size();
}

char Lexer::peekNext() const {
    if(pos_ + 1 >= source_.size()) return '\0';
    return source_[pos_ + 1];
}

Token Lexer::identifierOrKeyword() {
    int start = pos_ - 1;
    while(isalnum(peek()) || peek() == '_') advance();
    std::string_view lexeme = std::string_view(source_).substr(start, pos_ - start);
    if(lexeme == "packet") return Token{TokenType::Packet, lexeme, line_, col_ - (pos_ - start)};
    if(lexeme == "enum") return Token{TokenType::Enum, lexeme, line_, col_ - (pos_ - start)};
    if(lexeme == "uint8") return Token{TokenType::UInt8, lexeme, line_,col_ - (pos_ - start)};
    if(lexeme == "uint16") return Token{TokenType::UInt16, lexeme, line_, col_ - (pos_ - start)};
    if(lexeme == "uint32") return Token{TokenType::UInt32, lexeme, line_, col_ - (pos_ - start)};
    if(lexeme == "int8") return Token{TokenType::Int8, lexeme, line_, col_ - (pos_ - start)};
    if(lexeme == "int16") return Token{TokenType::Int16, lexeme, line_, col_ - (pos_ - start)};
    if(lexeme == "int32") return Token{TokenType::Int32, lexeme, line_, col_ - (pos_ - start)};
    if(lexeme == "float32") return Token{TokenType::Float32, lexeme, line_, col_ - (pos_ - start)};
    if(lexeme == "crc16") return Token{TokenType::Crc16, lexeme, line_, col_ - (pos_ - start)};
    return Token{TokenType::Identifier, lexeme, line_, col_ - (pos_ - start)};
}

Token Lexer::number() {
    int start = pos_ - 1;
    while(isdigit(peek())) advance();
    std::string_view lexeme = std::string_view(source_).substr(start, pos_ - start);
    return Token{TokenType::IntegerLiteral, lexeme, line_, col_ - (pos_ - start)};
}

void Lexer::skipWhitespaceAndComments() {
    while(true) {
        char c = peek();
        switch(c) {
            case ' ':
                advance();
                break;
            case '\r':
                advance();
                break;
            case '\t':
                advance();
                break;
            case '\n':
                advance();
                break;
            case '/':
                if(peekNext() == '/') {
                    while(peek() != '\n' && !isAtEnd()) advance();
                    break;
                } else if(peekNext() == '*') {
                    advance();
                    advance();
                    while(!isAtEnd() && !(peek() == '*' && peekNext() == '/')) advance();
                    if(!isAtEnd()) advance();
                    if(!isAtEnd()) advance();
                    break;
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while(true) {
        Token token = scanToken();
        tokens.push_back(token);
        if(token.type == TokenType::Eof) break;
    }
    return tokens;
}


