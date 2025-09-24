#ifndef PARSER_EXCEPTION_HPP
#define PARSER_EXCEPTION_HPP

#include <stdexcept>
#include <string>
#include "source_location.hpp"

class ParserException : public std::runtime_error {
public:
    const SourceLocation Location;

    ParserException(const std::string& msg, const SourceLocation& loc)
        : std::runtime_error(msg), Location(loc) {}
};

#endif