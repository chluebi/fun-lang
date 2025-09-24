#include <iostream>
#include <stdexcept>
#include <memory>

#include "runner.hpp"


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.lang>" << std::endl;
        return 1;
    }
    
    try {
        std::unique_ptr<InterpreterValue> result = runFile(argv[1]);

        // Print the result of the execution
        if (result) {
            if (auto longResult = dynamic_cast<InterpreterValueLong*>(result.get())) {
                std::cout << "Execution result: " << longResult->getValue() << std::endl;
            } else if (auto boolResult = dynamic_cast<InterpreterValueBool*>(result.get())) {
                std::cout << "Execution result: " << (boolResult->getValue() ? "true" : "false") << std::endl;
            } else {
                std::cerr << "Execution completed, but the result is of an unhandled type." << std::endl;
                return 1; // Indicate failure
            }
        } else {
            std::cerr << "Execution failed or returned null." << std::endl;
            return 1; // Indicate failure
        }

    } catch (const FileError& e) {
        std::cerr << "File Error: " << e.what() << std::endl;
        return 1;
    } catch (const ParsingError& e) {
        std::cerr << "Parsing Error: " << e.what() << std::endl;
        return 1;
    } catch (const EvaluationError& e) {
        std::cerr << "Evaluation Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        // Catch any other standard exceptions
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}