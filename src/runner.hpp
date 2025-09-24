#ifndef MAIN_HPP
#define MAIN_HPP

#include <string>
#include <memory>
#include "interpreter.hpp"

class FileError : public std::runtime_error {
public:
    explicit FileError(const std::string& message) : std::runtime_error(message) {}
};

class ParsingError : public std::runtime_error {
public:
    explicit ParsingError(const std::string& message) : std::runtime_error(message) {}
};

class EvaluationError : public std::runtime_error {
public:
    explicit EvaluationError(const std::string& message) : std::runtime_error(message) {}
};
std::string readFile(const std::string& filePath);
std::unique_ptr<InterpreterValue> runFile(char file[]);


#endif