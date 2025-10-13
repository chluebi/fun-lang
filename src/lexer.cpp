#include "lexer.hpp"

std::string tokenToString(Token token) {
    switch (token.Kind) {
        case TokenKind::Eof: return "Eof";
        case TokenKind::Fn: return "fn";
        case TokenKind::Let: return "let";
        case TokenKind::In: return "in";
        case TokenKind::Match: return "match";
        case TokenKind::LParen: return "(";
        case TokenKind::RParen: return ")";
        case TokenKind::LBrace: return "{";
        case TokenKind::RBrace: return "}";
        case TokenKind::LBracket: return "[";
        case TokenKind::RBracket: return "]";
        case TokenKind::Equal: return "==";
        case TokenKind::Comma: return ",";
        case TokenKind::Arrow: return "->";
        case TokenKind::Add: return "+";
        case TokenKind::Sub: return "-";
        case TokenKind::Mul: return "*";
        case TokenKind::Div: return "/";
        case TokenKind::Eq: return "==";
        case TokenKind::Neq: return "!=";
        case TokenKind::Leq: return "<=";
        case TokenKind::Lt: return "<";
        case TokenKind::Geq: return ">=";
        case TokenKind::Gt: return ">";
        case TokenKind::And: return "&&";
        case TokenKind::Or: return "||";
        case TokenKind::Identifier: return token.Text;
        case TokenKind::Number: return std::to_string(token.Value);
        case TokenKind::True: return "true";
        case TokenKind::False: return "false";
        case TokenKind::Unknown: return "Unknown";
        default: return "Unknown!!";
    }
}

void Lexer::parseIdentifierOrKeyword() {
    size_t startPos = CurrentPos;
    std::string text;
    char c = peek();
    while (isIdentifierChar(c)) {
        text += c;
        consume();
        c = peek();
    }

    TokenKind kind;
    if (text == "fn") kind = TokenKind::Fn;
    else if (text == "let") kind = TokenKind::Let;
    else if (text == "in") kind = TokenKind::In;
    else if (text == "match") kind = TokenKind::Match;
    else if (text == "true") kind = TokenKind::True;
    else if (text == "false") kind = TokenKind::False;
    else kind = TokenKind::Identifier;

    CurrentToken = Token{kind, text, -1, SourceLocation{startPos, CurrentPos}};
}

void Lexer::parseNumber() {
    size_t startPos = CurrentPos;
    std::string text;
    char c = peek();
    while (isdigit(c)) {
        text += c;
        consume();
        c = peek();
    }
    CurrentToken = Token{TokenKind::Number, text, std::stol(text), SourceLocation{startPos, CurrentPos}};
}

void Lexer::parseOperator() {
    size_t startPos = CurrentPos;
    std::string text;
    char c1 = consume();
    char c2 = peek();
    text += c1;

    TokenKind kind;
    if (c1 == '(') kind = TokenKind::LParen;
    else if (c1 == ')') kind = TokenKind::RParen;
    else if (c1 == '{') kind = TokenKind::LBrace;
    else if (c1 == '}') kind = TokenKind::RBrace;
    else if (c1 == '[') kind = TokenKind::LBracket;
    else if (c1 == ']') kind = TokenKind::RBracket;
    else if (c1 == ',') kind = TokenKind::Comma;
    else if (c1 == '+') kind = TokenKind::Add;
    else if (c1 == '-') {
        if (c2 == '>') {
            text += consume();
            kind = TokenKind::Arrow;
        } else {
            kind = TokenKind::Sub;
        }
    }
    else if (c1 == '*') kind = TokenKind::Mul;
    else if (c1 == '/') kind = TokenKind::Div;
    else if (c1 == '=') {
        if (c2 == '=') {
            text += consume();
            kind = TokenKind::Eq;
        } else {
            kind = TokenKind::Equal;
        }
    } else if (c1 == '>') {
        if (c2 == '=') {
            text += consume();
            kind = TokenKind::Geq;
        } else {
            kind = TokenKind::Gt;
        }
    } else if (c1 == '<') {
        if (c2 == '=') {
            text += consume();
            kind = TokenKind::Leq;
        } else {
            kind = TokenKind::Lt;
        }
    } else if (c1 == '!') {
        if (c2 == '=') {
            text += consume();
            kind = TokenKind::Neq;
        } else {
            throw LexerException("Unrecognized token '!'", SourceLocation{startPos, CurrentPos});
        }
    } else if (c1 == '&') {
        if (c2 == '&') {
            text += consume();
            kind = TokenKind::And;
        } else {
            throw LexerException("Unrecognized token '&'", SourceLocation{startPos, CurrentPos});
        }
    } else if (c1 == '|') {
        if (c2 == '|') {
            text += consume();
            kind = TokenKind::Or;
        } else {
            throw LexerException("Unrecognized token '|'", SourceLocation{startPos, CurrentPos});
        }
    } else {
        throw LexerException("Unrecognized token '" + std::string(1, c1) + "'", SourceLocation{startPos, CurrentPos});
    }

    CurrentToken = Token{kind, text, -1, SourceLocation{startPos, CurrentPos}};
}
