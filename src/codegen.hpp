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

class CodegenContext {
public:
    std::map<std::string, llvm::Value *> NamedValues;
};

class CodeGenerator: AstLLVMValueVisitor {
public:
    std::unique_ptr<llvm::LLVMContext> TheContext;
    std::unique_ptr<llvm::Module> TheModule;
    std::unique_ptr<llvm::IRBuilder<>> Builder;
public:
    llvm::Value *codegen(const AstExpr& expr, CodegenContext& ctx) const;
    llvm::Value *codegen(const AstFunction& func, CodegenContext& ctx);
    llvm::Value *codegenPrintResult(const AstFunction& func, CodegenContext& ctx);
private:
    llvm::Value *visit(const AstExprConstLong& expr, CodegenContext& ctx) const override;
    llvm::Value *visit(const AstExprConstBool& expr, CodegenContext& ctx) const override;
    llvm::Value *visit(const AstExprConstArray &expr, CodegenContext &ctx) const override;
    llvm::Value *visit(const AstExprVariable &expr, CodegenContext &ctx) const override;
    llvm::Value *visit(const AstExprIndex &expr, CodegenContext &ctx) const override;
    llvm::Value *visit(const AstExprCall& expr, CodegenContext& ctx) const override;
    llvm::Value *visit(const AstExprLetIn& expr, CodegenContext& ctx) const override;
    llvm::Value *visit(const AstExprMatch& expr, CodegenContext& ctx) const override;

    llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>& expr, CodegenContext& ctx) const override;
    llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>& expr, CodegenContext& ctx) const override;
    llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>& expr, CodegenContext& ctx) const override;
    llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>& expr, CodegenContext& ctx) const override;

    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>& expr, CodegenContext& ctx) const override;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>& expr, CodegenContext& ctx) const override;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>& expr, CodegenContext& ctx) const override;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>& expr, CodegenContext& ctx) const override;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>& expr, CodegenContext& ctx) const override;
    llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>& expr, CodegenContext& ctx) const override;

    llvm::Value *visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>& expr, CodegenContext& ctx) const override;
    llvm::Value *visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>& expr, CodegenContext& ctx) const override;
};


#endif