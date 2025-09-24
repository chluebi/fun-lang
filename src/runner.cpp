#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>

#include "interpreter.hpp"
#include "parser.hpp"
#include "lexer.hpp"
#include "runner.hpp"
#include "lexer_exception.hpp"
#include "parser_exception.hpp"
#include <filesystem>

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

// Helper function to get line and column information from a position
std::pair<size_t, size_t> getLineAndCol(const std::string& source, size_t pos) {
    size_t line = 1;
    size_t col = 1;
    for (size_t i = 0; i < pos && i < source.length(); ++i) {
        if (source[i] == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
    }
    return {line, col};
}

// Helper function to extract and print affected lines
void printAffectedCode(const std::string& source, const SourceLocation& loc, const std::string& filePath) {
    auto [line, col] = getLineAndCol(source, loc.StartPos);
    
    std::cout << filePath << ":" << line << ":" << col << std::endl;

    size_t lineStart = source.rfind('\n', loc.StartPos);
    if (lineStart == std::string::npos) {
        lineStart = 0;
    } else {
        lineStart += 1;
    }

    size_t lineEnd = source.find('\n', loc.EndPos);
    if (lineEnd == std::string::npos) {
        lineEnd = source.length();
    }

    std::string affectedCode = source.substr(lineStart, lineEnd - lineStart);
    std::cout << affectedCode << std::endl;

    size_t currentLineStart = lineStart;
    for (char c : affectedCode) {
        if (c == '\n') {
            currentLineStart++;
            continue;
        }
        if (currentLineStart >= loc.StartPos && currentLineStart < loc.EndPos) {
            std::cout << "^";
        } else {
            std::cout << " ";
        }
        currentLineStart++;
    }
    std::cout << std::endl;
}


std::unique_ptr<InterpreterValue> runFile(char file[]) {
    std::string sourceCode = readFile(file);

    Parser parser(sourceCode);
    Context globalContext;

    while (parser.get().Kind == TokenKind::Fn) {
        auto func = parser.parseFunction();
        if (!func) {
            throw ParsingError("Parsing failed while defining a function.");
        }
        globalContext.addFunction(std::move(func));
    }
    
    auto resultExpr = parser.parseExpression();

    if (!resultExpr) {
        throw ParsingError("Parsing failed for the main expression.");
    }

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