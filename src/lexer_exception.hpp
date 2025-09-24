#ifndef LEXER_EXCEPTION_HPP
#define LEXER_EXCEPTION_HPP

#include <stdexcept>
#include <string>
#include "source_location.hpp"

class LexerException : public std::runtime_error {
public:
    const SourceLocation Location;

    LexerException(const std::string& msg, const SourceLocation& loc)
        : std::runtime_error(msg), Location(loc) {}
};

#endif