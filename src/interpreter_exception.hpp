#ifndef INTERPRETER_EXCEPTION_HPP
#define INTERPRETER_EXCEPTION_HPP

#include <stdexcept>
#include <string>
#include "source_location.hpp"

class InterpreterException : public std::runtime_error {
public:
    const SourceLocation Location;

    InterpreterException(const std::string& msg, const SourceLocation& loc)
        : std::runtime_error(msg), Location(loc) {}
};

class UndefinedVariableException : public InterpreterException {
public:
    UndefinedVariableException(const std::string& name, const SourceLocation& loc)
        : InterpreterException("Undefined variable: '" + name + "'", loc) {}
};

class UndefinedFunctionException : public InterpreterException {
public:
    UndefinedFunctionException(const std::string& name, const SourceLocation& loc)
        : InterpreterException("Undefined function: '" + name + "'", loc) {}
};

class TypeMismatchException : public InterpreterException {
public:
    TypeMismatchException(const std::string& msg, const SourceLocation& loc)
        : InterpreterException("Type mismatch: " + msg, loc) {}
};

class ArityMismatchException : public InterpreterException {
public:
    ArityMismatchException(const std::string& name, size_t expected, size_t actual, const SourceLocation& loc)
        : InterpreterException("Function '" + name + "' called with wrong number of arguments. Expected " + std::to_string(expected) + ", got " + std::to_string(actual), loc) {}
};

class DivisionByZeroException : public InterpreterException {
public:
    DivisionByZeroException(const SourceLocation& loc)
        : InterpreterException("Division by zero", loc) {}
};

class NoMatchFoundException : public InterpreterException {
public:
    NoMatchFoundException(const SourceLocation& loc)
        : InterpreterException("Match expression exhausted without finding a matching path", loc) {}
};

class IndexOutOfBoundsException : public InterpreterException {
public:
    IndexOutOfBoundsException(const SourceLocation& loc)
        : InterpreterException("Index out of bounds", loc) {}
};

#endif