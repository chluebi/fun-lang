#include <iostream>
#include <string>
#include <stdexcept>

#include "runner.hpp"
#include "lexer_exception.hpp"
#include "parser_exception.hpp"
#include "interpreter_exception.hpp"


std::string readFile(const std::string& filePath);
void printAffectedCode(const std::string& source, const SourceLocation& loc, const std::string& filePath);


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    try {
        std::unique_ptr<InterpreterValue> result = runFile(argv[1]);
        if (result) {
            if (auto longResult = dynamic_cast<InterpreterValueLong*>(result.get())) {
                std::cout << "Execution result: " << longResult->getValue() << std::endl;
            } else if (auto boolResult = dynamic_cast<InterpreterValueBool*>(result.get())) {
                std::cout << "Execution result: " << (boolResult->getValue() ? "true" : "false") << std::endl;
            }
        }
    } catch (const LexerException& e) {
        std::cerr << "Lexer Error: " << e.what() << std::endl;
        try {
            std::string sourceCode = readFile(argv[1]);
            printAffectedCode(sourceCode, e.Location, argv[1]);
        } catch (const std::exception& fileE) {
            std::cerr << "Could not read file for error display: " << fileE.what() << std::endl;
        }
        return 1;
    } catch (const ParserException& e) {
        std::cerr << "Parser Error: " << e.what() << std::endl;
        try {
            std::string sourceCode = readFile(argv[1]);
            printAffectedCode(sourceCode, e.Location, argv[1]);
        } catch (const std::exception& fileE) {
            std::cerr << "Could not read file for error display: " << fileE.what() << std::endl;
        }
        return 1;
    } catch (const InterpreterException& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        try {
            std::string sourceCode = readFile(argv[1]);
            printAffectedCode(sourceCode, e.Location, argv[1]);
        } catch (const std::exception& fileE) {
            std::cerr << "Could not read file for error display: " << fileE.what() << std::endl;
        }
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}