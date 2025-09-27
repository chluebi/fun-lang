#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include <map>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "ast.hpp"
#include "codegen_exception.hpp"


class CodeGenerator : AstLLVMValueVisitor {
    std::unique_ptr<llvm::LLVMContext> TheContext;
    std::unique_ptr<llvm::Module> TheModule;
    std::unique_ptr<llvm::IRBuilder<>> Builder;
    std::map<std::string, llvm::Value *> NamedValues;
public:
    llvm::Value *codegen(const AstExpr& expr) const;
private:
    llvm::Value *visit(const AstExprConstLong& expr) const = 0;
    llvm::Value *visit(const AstExprConstBool& expr) const = 0;
    llvm::Value *visit(const AstExprVariable& expr) const = 0;
    llvm::Value *visit(const AstExprCall& expr) const = 0;
    llvm::Value *visit(const AstExprLetIn& expr) const = 0;
    llvm::Value *visit(const AstExprMatch& expr) const = 0;

    llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>& expr) const = 0;
    llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>& expr) const = 0;
    llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>& expr) const = 0;
    llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>& expr) const = 0;

    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>& expr) const = 0;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>& expr) const = 0;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>& expr) const = 0;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>& expr) const = 0;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>& expr) const = 0;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>& expr) const = 0;

    llvm::Value *visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>& expr) const = 0;
    llvm::Value *visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>& expr) const = 0;
};


#endif