#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <optional>
#include <iostream>

#include "source_location.hpp"
#include "lexer_exception.hpp"

enum class TokenKind {
    Eof,
    Fn, Let, In, Match,
    LParen, RParen, LBrace, RBrace,
    Equal, Comma, Arrow,
    Add, Sub, Mul, Div,
    Eq, Neq, Leq, Lt, Geq, Gt,
    And, Or,
    Identifier,
    Number,
    True, False,
    Unknown
};

struct Token {
    TokenKind Kind;
    std::string Text;
    long Value;
    SourceLocation Location;
};

std::string tokenToString(Token kind);

class Lexer {
    const std::string& Input;
    size_t CurrentPos = 0;
    std::optional<Token> CurrentToken;

    char peek() {
        if (CurrentPos >= Input.length()) return 0;
        return Input[CurrentPos];
    }
    char consume() {
        if (CurrentPos >= Input.length()) return 0;
        return Input[CurrentPos++];
    }
    void skipWhitespace() {
        while (isspace(peek())) consume();
    }
    bool isIdentifierChar(char c) {
        return isalnum(c) || c == '_';
    }

    void parseIdentifierOrKeyword();
    void parseNumber();
    void parseOperator();

public:
    Lexer(const std::string& input) : Input(input) {
        nextToken();
    }

    const Token& get() const {
        return *CurrentToken;
    }

    size_t getCurrentPosition() {
        return CurrentPos;
    }

    Token nextToken() {
        skipWhitespace();
        if (CurrentPos >= Input.length()) {
            CurrentToken = Token{TokenKind::Eof, "", -1, SourceLocation{CurrentPos, CurrentPos}};
            return *CurrentToken;
        }

        char c = peek();

        if (isalpha(c) || c == '_') {
            parseIdentifierOrKeyword();
        } else if (isdigit(c)) {
            parseNumber();
        } else {
            parseOperator();
        }
        return *CurrentToken;
    }
};

#endif