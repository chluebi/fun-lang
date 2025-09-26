#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <memory>
#include <utility>

#include "interpreter.hpp"
#include "parser.hpp"
#include "lexer.hpp"
#include "runner.hpp"
#include "lexer_exception.hpp"
#include "parser_exception.hpp"
#include "interpreter_exception.hpp"


std::string readFile(const std::string& filePath) {
    std::ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        throw FileError("Error: Could not open file " + filePath);
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

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
    std::string filePath = file;
    std::string sourceCode = readFile(filePath);

    Parser parser(sourceCode);
    Context globalContext;

    while (parser.get().Kind == TokenKind::Fn) {
        auto func = parser.parseFunction();
        if (!func) {
            throw ParserException("Parsing failed while defining a function.", SourceLocation{0, 0});
        }
        globalContext.addFunction(std::move(func));
    }
    
    auto resultExpr = parser.parseExpression();

    if (!resultExpr) {
        throw ParserException("Parsing failed for the main expression.", parser.get().Location);
    }
    
    Interpreter interpreter = Interpreter(globalContext);
    std::unique_ptr<InterpreterValue> result = interpreter.eval(*resultExpr);

    if (result) {
        if (dynamic_cast<InterpreterValueLong*>(result.get()) || dynamic_cast<InterpreterValueBool*>(result.get())) {
            return result;
        } else {
            throw InterpreterException("Execution completed, but the result is of an unexpected internal type.", resultExpr->getLocation());
        }
    } else {
        throw InterpreterException("Interpreter::eval returned null unexpectedly.", resultExpr->getLocation());
    }
}