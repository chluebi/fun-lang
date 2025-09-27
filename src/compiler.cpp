#include "codegen.hpp"
#include "runner.hpp"
#include "parser.hpp"


int compileFile(char file[]) {
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
        codeGenerator.codegen(*func);
    }

    codeGenerator.TheModule->print(llvm::errs(), nullptr);

    return 0;
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    return compileFile(argv[1]);
}