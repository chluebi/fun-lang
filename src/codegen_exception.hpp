#ifndef CODEGEN_EXCEPTION_HPP
#define CODEGEN_EXCEPTION_HPP

#include <stdexcept>
#include <string>
#include "source_location.hpp"

class CodegenException : public std::runtime_error {
public:
    const SourceLocation Location;

    CodegenException(const std::string& msg, const SourceLocation& loc)
        : std::runtime_error(msg), Location(loc) {}
};

#endif