#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

#include "interpreter.hpp"
#include "parser.hpp"
#include "lexer.hpp"
#include "runner.hpp"

// A simple utility function to read the entire content of a file into a string
std::string readFile(const std::string& filePath) {
    std::ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        throw FileError("Error: Could not open file " + filePath);
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

std::unique_ptr<InterpreterValue> runFile(char file[]) {
    std::string sourceCode = readFile(file);
    
    // Create a parser instance with the source code
    Parser parser(sourceCode);
    Context globalContext;

    // First, parse all function definitions.
    while (parser.get().Kind == TokenKind::Fn) {
        auto func = parser.parseFunction();
        if (!func) {
            throw ParsingError("Parsing failed while defining a function.");
        }
        globalContext.addFunction(std::move(func));
    }
    
    // After all function definitions are parsed, parse the final top-level expression.
    auto resultExpr = parser.parseExpression();

    if (!resultExpr) {
        throw ParsingError("Parsing failed for the main expression.");
    }

    // Evaluate the final expression using the global context.
    std::unique_ptr<InterpreterValue> result = resultExpr->eval(globalContext);

    if (result) {
        if (dynamic_cast<InterpreterValueLong*>(result.get()) || dynamic_cast<InterpreterValueBool*>(result.get())) {
            return result;
        } else {
            throw EvaluationError("Execution completed, but the result is of an unhandled type.");
        }
    } else {
        throw EvaluationError("Execution failed or returned null.");
    }
}