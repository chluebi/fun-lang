#include "codegen.hpp"

llvm::Value *CodeGenerator::codegen(const AstExpr& expr) const {
    return expr.accept(*this);
}

llvm::Value *CodeGenerator::codegen(const AstFunction& func) {
    llvm::Function *TheFunction = TheModule->getFunction(func.getPrototype()->getName());

    std::vector<llvm::Type *> ArgTypes;
    for (auto arg : func.getPrototype()->getArgs()) {
        ArgTypes.push_back(llvm::Type::getInt64Ty(*TheContext));
    }

    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt64Ty(*TheContext), ArgTypes, false);

    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, func.getPrototype()->getName(), TheModule.get());

    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(func.getPrototype()->getArgs()[Idx++].Name);

    TheFunction = F;

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);

    NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        NamedValues[std::string(Arg.getName())] = &Arg;

    if (llvm::Value *RetVal = this->codegen(*func.getBody())) {
        Builder->CreateRet(RetVal);

        verifyFunction(*TheFunction);

        return TheFunction;
    }
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


llvm::Value *CodeGenerator::visit(const AstExprCall& expr) const {
    throw CodegenException("AstExprCall codegen not implemented", expr.getLocation());
}

llvm::Value *CodeGenerator::visit(const AstExprLetIn& expr) const {
    throw CodegenException("AstExprLetIn codegen not implemented", expr.getLocation());
}

llvm::Value *CodeGenerator::visit(const AstExprMatch& expr) const {
    throw CodegenException("AstExprMatch codegen not implemented", expr.getLocation());
}

// --- Arithmetic Binary Operations (Int to Int) ---

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>& expr) const {
    throw CodegenException("AstExprBinaryIntToInt::Add codegen not implemented", expr.getLocation());
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>& expr) const {
    throw CodegenException("AstExprBinaryIntToInt::Sub codegen not implemented", expr.getLocation());
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>& expr) const {
    throw CodegenException("AstExprBinaryIntToInt::Mul codegen not implemented", expr.getLocation());
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>& expr) const {
    throw CodegenException("AstExprBinaryIntToInt::Div codegen not implemented", expr.getLocation());
}

// --- Comparison Binary Operations (Int to Bool) ---

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>& expr) const {
    throw CodegenException("AstExprBinaryIntToBool::Eq codegen not implemented", expr.getLocation());
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>& expr) const {
    throw CodegenException("AstExprBinaryIntToBool::Neq codegen not implemented", expr.getLocation());
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>& expr) const {
    throw CodegenException("AstExprBinaryIntToBool::Leq codegen not implemented", expr.getLocation());
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>& expr) const {
    throw CodegenException("AstExprBinaryIntToBool::Lt codegen not implemented", expr.getLocation());
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>& expr) const {
    throw CodegenException("AstExprBinaryIntToBool::Geq codegen not implemented", expr.getLocation());
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>& expr) const {
    throw CodegenException("AstExprBinaryIntToBool::Gt codegen not implemented", expr.getLocation());
}

// --- Logical Binary Operations (Bool to Bool) ---

llvm::Value *CodeGenerator::visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>& expr) const {
    throw CodegenException("AstExprBinaryBoolToBool::And codegen not implemented", expr.getLocation());
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>& expr) const {
    throw CodegenException("AstExprBinaryBoolToBool::Or codegen not implemented", expr.getLocation());
}

