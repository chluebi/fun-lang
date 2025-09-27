#include "codegen.hpp"

llvm::Value *CodeGenerator::codegen(const AstExpr& expr) const {
    return expr.accept(*this);
}

llvm::Value *CodeGenerator::visit(const AstExprConstLong& expr) const {
    llvm::Type* Int64Ty = llvm::Type::getInt64Ty(*TheContext); 
    return llvm::ConstantInt::get(Int64Ty, expr.getValue(), true /* isSigned */); 
}

llvm::Value *CodeGenerator::visit(const AstExprConstBool& expr) const {
    llvm::Type* Int1Ty = llvm::Type::getInt1Ty(*TheContext); 
    return llvm::ConstantInt::get(Int1Ty, expr.getValue()); 
}


llvm::Value *CodeGenerator::visit(const AstExprVariable& expr) const {
  auto it = NamedValues.find(expr.getName());

  if (it == NamedValues.end()) {
    throw CodegenException("Variable not found", expr.getLocation());
  }
  
  return it->second;
}