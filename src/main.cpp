#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <vector>

#include "interpreter.hpp"
#include "parser.hpp"
#include "lexer.hpp"

// A simple utility function to read the entire content of a file into a string
std::string readFile(const std::string& filePath) {
    std::ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.lang>" << std::endl;
        return 1;
    }

    std::string sourceCode = readFile(argv[1]);
    if (sourceCode.empty()) {
        return 1;
    }

    // Create a parser instance with the source code
    Parser parser(sourceCode);
    Context globalContext;

    std::cout << "Parsing file..." << std::endl;

    // First, parse all function definitions.
    // The parser consumes 'fn' inside `parseFunction`.
    while (parser.get().Kind == TokenKind::Fn) {
        auto func = parser.parseFunction();
        if (!func) {
            std::cerr << "Parsing failed. Exiting." << std::endl;
            return 1;
        }
        globalContext.addFunction(std::move(func));
    }
    
    // After all function definitions are parsed, parse the final top-level expression.
    auto resultExpr = parser.parseExpression();

    if (!resultExpr) {
        std::cerr << "Parsing failed. Exiting." << std::endl;
        return 1;
    }

    std::cout << "Parsing successful." << std::endl;

    // Evaluate the final expression using the global context.
    std::cout << "Executing program..." << std::endl;
    std::unique_ptr<InterpreterValue> result = resultExpr->eval(globalContext);

    // Print the result of the execution
    if (result) {
        if (auto longResult = dynamic_cast<InterpreterValueLong*>(result.get())) {
            std::cout << "Execution result: " << longResult->getValue() << std::endl;
        } else if (auto boolResult = dynamic_cast<InterpreterValueBool*>(result.get())) {
            std::cout << "Execution result: " << (boolResult->getValue() ? "true" : "false") << std::endl;
        } else {
            std::cerr << "Execution completed, but the result is of an unhandled type." << std::endl;
        }
    } else {
        std::cerr << "Execution failed or returned null." << std::endl;
    }

    return 0;
}