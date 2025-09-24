#include <iostream>
#include <string>

#include "runner.hpp"
#include "lexer_exception.hpp"
#include "parser_exception.hpp"


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
            } else {
                std::cerr << "Execution completed, but the result is of an unhandled type." << std::endl;
                return 1;
            }
        }
    } catch (const LexerException& e) {
        std::cerr << "Lexer Error: " << e.what() << std::endl;
        std::string sourceCode = readFile(argv[1]);
        printAffectedCode(sourceCode, e.Location, argv[1]);
    } catch (const ParserException& e) {
        std::cerr << "Parser Error: " << e.what() << std::endl;
        std::string sourceCode = readFile(argv[1]);
        printAffectedCode(sourceCode, e.Location, argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}