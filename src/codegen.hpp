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
public:
    std::unique_ptr<llvm::LLVMContext> TheContext;
    std::unique_ptr<llvm::Module> TheModule;
    std::unique_ptr<llvm::IRBuilder<>> Builder;
    std::map<std::string, llvm::Value *> NamedValues;
public:
    llvm::Value *codegen(const AstExpr& expr) const;
    llvm::Value *codegen(const AstFunction& func);
private:
    llvm::Value *visit(const AstExprConstLong& expr) const override;
    llvm::Value *visit(const AstExprConstBool& expr) const override;
    llvm::Value *visit(const AstExprVariable& expr) const override;
    llvm::Value *visit(const AstExprCall& expr) const override;
    llvm::Value *visit(const AstExprLetIn& expr) const override;
    llvm::Value *visit(const AstExprMatch& expr) const override;

    llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>& expr) const override;
    llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>& expr) const override;
    llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>& expr) const override;
    llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>& expr) const override;

    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>& expr) const override;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>& expr) const override;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>& expr) const override;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>& expr) const override;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>& expr) const override;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>& expr) const override;

    llvm::Value *visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>& expr) const override;
    llvm::Value *visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>& expr) const override;
};


#endif