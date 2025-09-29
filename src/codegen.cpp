#include "codegen.hpp"

llvm::Value *CodeGenerator::codegen(const AstExpr& expr, CodegenContext& ctx) const {
    return expr.accept(*this, ctx);
}

llvm::Value *CodeGenerator::codegen(const AstFunction& func, CodegenContext& ctx) {
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

    ctx.NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        ctx.NamedValues[std::string(Arg.getName())] = &Arg;

    if (llvm::Value *RetVal = this->codegen(*func.getBody(), ctx)) {
        Builder->CreateRet(RetVal);

        verifyFunction(*TheFunction);

        return TheFunction;
    }
}


llvm::Value *CodeGenerator::visit(const AstExprConstLong& expr, CodegenContext& ctx) const {
    llvm::Type* Int64Ty = llvm::Type::getInt64Ty(*TheContext); 
    return llvm::ConstantInt::get(Int64Ty, expr.getValue(), true /* isSigned */); 
}

llvm::Value *CodeGenerator::visit(const AstExprConstBool& expr, CodegenContext& ctx) const {
    llvm::Type* Int1Ty = llvm::Type::getInt1Ty(*TheContext); 
    return llvm::ConstantInt::get(Int1Ty, expr.getValue()); 
}


llvm::Value *CodeGenerator::visit(const AstExprVariable& expr, CodegenContext& ctx) const {
    auto it = ctx.NamedValues.find(expr.getName());

    if (it == ctx.NamedValues.end()) {
      throw CodegenException("Variable not found", expr.getLocation());
    }
    
    return it->second;
}


llvm::Value *CodeGenerator::visit(const AstExprCall& expr, CodegenContext& ctx) const {
    llvm::Function *CalleeF = TheModule->getFunction(expr.getCallee());
    if (!CalleeF)
        throw CodegenException("Function " + expr.getCallee() + " not found", expr.getLocation());
    
    if (CalleeF->arg_size() != expr.getArgs().size())
        throw CodegenException("Wrong number of args", expr.getLocation());

    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = expr.getArgs().size(); i != e; ++i) {
        ArgsV.push_back(this->codegen(*expr.getArgs()[i], ctx));
        if (!ArgsV.back())
            throw CodegenException("Arg returned nullptr", (*expr.getArgs()[i]).getLocation());
    }

    return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Value *CodeGenerator::visit(const AstExprLetIn& expr, CodegenContext& ctx) const {
    llvm::Value *RHS = this->codegen(*expr.getExpr(), ctx);
    if (!RHS) {
        return nullptr;
    }

    auto oldBinding = ctx.NamedValues.find(expr.getVariable());
    llvm::Value *oldVal = nullptr;
    if (oldBinding != ctx.NamedValues.end()) {
        oldVal = oldBinding->second;
    }

    ctx.NamedValues[expr.getVariable()] = RHS; 
    llvm::Value *BodyVal = this->codegen(*expr.getBody(), ctx);
    
    if (oldBinding != ctx.NamedValues.end()) {
        ctx.NamedValues[expr.getVariable()] = oldVal;
    } else {
        ctx.NamedValues.erase(expr.getVariable());
    }

    return BodyVal;
}

llvm::Value *CodeGenerator::visit(const AstExprMatch& expr, CodegenContext& ctx) const {
    throw CodegenException("AstExprMatch codegen not implemented", expr.getLocation());
}

// --- Arithmetic Binary Operations (Int to Int) ---

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>& expr, CodegenContext& ctx) const {
    llvm::Value *L = this->codegen(*expr.getLHS(), ctx);
    llvm::Value *R = this->codegen(*expr.getRHS(), ctx);
    return Builder->CreateAdd(L, R, "addtmp");
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>& expr, CodegenContext& ctx) const {
    llvm::Value *L = this->codegen(*expr.getLHS(), ctx);
    llvm::Value *R = this->codegen(*expr.getRHS(), ctx);
    return Builder->CreateSub(L, R, "subtmp");
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>& expr, CodegenContext& ctx) const {
    llvm::Value *L = this->codegen(*expr.getLHS(), ctx);
    llvm::Value *R = this->codegen(*expr.getRHS(), ctx);
    return Builder->CreateMul(L, R, "multmp");
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>& expr, CodegenContext& ctx) const {
    llvm::Value *L = this->codegen(*expr.getLHS(), ctx);
    llvm::Value *R = this->codegen(*expr.getRHS(), ctx);
    return Builder->CreateSDiv(L, R, "divtmp");
}

// --- Comparison Binary Operations (Int to Bool) ---

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>& expr, CodegenContext& ctx) const {
    llvm::Value *L = this->codegen(*expr.getLHS(), ctx);
    llvm::Value *R = this->codegen(*expr.getRHS(), ctx);
    return Builder->CreateICmpEQ(L, R, "eqtmp");
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>& expr, CodegenContext& ctx) const {
    llvm::Value *L = this->codegen(*expr.getLHS(), ctx);
    llvm::Value *R = this->codegen(*expr.getRHS(), ctx);
    return Builder->CreateICmpNE(L, R, "neqtmp");
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>& expr, CodegenContext& ctx) const {
    llvm::Value *L = this->codegen(*expr.getLHS(), ctx);
    llvm::Value *R = this->codegen(*expr.getRHS(), ctx);
    return Builder->CreateICmpSLE(L, R, "leqtmp");
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>& expr, CodegenContext& ctx) const {
    llvm::Value *L = this->codegen(*expr.getLHS(), ctx);
    llvm::Value *R = this->codegen(*expr.getRHS(), ctx);
    return Builder->CreateICmpSLT(L, R, "lttmp");
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>& expr, CodegenContext& ctx) const {
    llvm::Value *L = this->codegen(*expr.getLHS(), ctx);
    llvm::Value *R = this->codegen(*expr.getRHS(), ctx);
    return Builder->CreateICmpSGE(L, R, "getmp");
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>& expr, CodegenContext& ctx) const {
    llvm::Value *L = this->codegen(*expr.getLHS(), ctx);
    llvm::Value *R = this->codegen(*expr.getRHS(), ctx);
    return Builder->CreateICmpSGT(L, R, "gtmp");
}

// --- Logical Binary Operations (Bool to Bool) ---

llvm::Value *CodeGenerator::visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>& expr, CodegenContext& ctx) const {
    llvm::Value *L = this->codegen(*expr.getLHS(), ctx);
    llvm::Value *R = this->codegen(*expr.getRHS(), ctx);
    return Builder->CreateAnd(L, R, "andtmp");
}

llvm::Value *CodeGenerator::visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>& expr, CodegenContext& ctx) const {
    llvm::Value *L = this->codegen(*expr.getLHS(), ctx);
    llvm::Value *R = this->codegen(*expr.getRHS(), ctx);
    return Builder->CreateOr(L, R, "ortmp");
}

