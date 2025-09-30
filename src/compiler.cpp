#include <fstream>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ToolOutputFile.h>

#include "codegen.hpp"
#include "runner.hpp"
#include "parser.hpp"


int compileFile(char file[], char outputFilename[]) {
    std::string filePath = file;
    std::string sourceCode = readFile(filePath);

    Parser parser(sourceCode);
    
    CodeGenerator codeGenerator;
    codeGenerator.TheContext = std::make_unique<llvm::LLVMContext>();
    codeGenerator.TheModule = std::make_unique<llvm::Module>("testcompiled", *codeGenerator.TheContext);
    codeGenerator.Builder = std::make_unique<llvm::IRBuilder<>>(*codeGenerator.TheContext);

    while (parser.get().Kind == TokenKind::Fn) {
        auto func = parser.parseFunction();
        if (!func) {
            throw ParserException("Parsing failed while defining a function.", SourceLocation{0, 0});
        }
        CodegenContext ctxt;
        codeGenerator.codegen(*func, ctxt);
    }

    auto resultExpr = parser.parseExpression();

    if (!resultExpr) {
        throw ParserException("Parsing failed for the main expression.", parser.get().Location);
    }

    auto mainFuncProto = std::make_unique<AstPrototype>(resultExpr->getLocation(), "main", std::vector<AstArg>{});
    auto resultFunction = std::make_unique<AstFunction>(resultExpr->getLocation(), std::move(mainFuncProto), std::move(resultExpr));

    CodegenContext ctxt;
    codeGenerator.codegenPrintResult(*resultFunction, ctxt);

    std::error_code EC;
    
    llvm::ToolOutputFile Out(outputFilename, EC, llvm::sys::fs::OF_None);

    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message() << "\n";
        return 1;
    }

    codeGenerator.TheModule->print(Out.os(), nullptr);
    
    Out.keep(); 
    
    llvm::outs() << "Successfully generated LLVM IR file: " << outputFilename << "\n";

    return 0;
}

int compileFileAndPrint(char file[], char outputFilename[]) {
    try {
        compileFile(file, outputFilename);
    } catch (const LexerException& e) {
        std::cerr << "Lexer Error: " << e.what() << std::endl;
        try {
            std::string sourceCode = readFile(file);
            printAffectedCode(sourceCode, e.Location, file);
        } catch (const std::exception& fileE) {
            std::cerr << "Could not read file for error display: " << fileE.what() << std::endl;
        }
        return 1;
    } catch (const ParserException& e) {
        std::cerr << "Parser Error: " << e.what() << std::endl;
        try {
            std::string sourceCode = readFile(file);
            printAffectedCode(sourceCode, e.Location, file);
        } catch (const std::exception& fileE) {
            std::cerr << "Could not read file for error display: " << fileE.what() << std::endl;
        }
        return 1;
    } catch (const CodegenException& e) {
        std::cerr << "Codegen Error: " << e.what() << std::endl;
        try {
            std::string sourceCode = readFile(file);
            printAffectedCode(sourceCode, e.Location, file);
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


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input filename> <output filename>" << std::endl;
        return 1;
    }

    return compileFileAndPrint(argv[1], argv[2]);
}