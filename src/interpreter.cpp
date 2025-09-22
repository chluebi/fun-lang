#include "interpreter.hpp"

std::unique_ptr<InterpreterValue> InterpreterValueLong::clone() const {
    return std::make_unique<InterpreterValueLong>(Value);
}

std::unique_ptr<InterpreterValue> InterpreterValueBool::clone() const {
    return std::make_unique<InterpreterValueBool>(Value);
}

std::unique_ptr<InterpreterValue> AstExprConst::eval(const Context& context) const {
    (void) context; // silence unused parameter
    return getValue();
}

std::unique_ptr<InterpreterValue> AstExprConstLong::getValue() const {
    return std::make_unique<InterpreterValueLong>(Value);
}

std::unique_ptr<AstExpr> AstExprConstLong::clone() const {
    return std::make_unique<AstExprConstLong>(Value);
}

std::unique_ptr<InterpreterValue> AstExprConstBool::getValue() const {
    return std::make_unique<InterpreterValueBool>(Value);
}

std::unique_ptr<AstExpr> AstExprConstBool::clone() const {
    return std::make_unique<AstExprConstBool>(Value);
}

AstFunction::AstFunction(std::unique_ptr<AstPrototype> Proto, std::unique_ptr<AstExpr> Body)
    : Proto(std::move(Proto)), Body(std::move(Body)) {}

std::unique_ptr<InterpreterValue> AstFunction::eval(const Context& context) const {
    return Body->eval(context);
}

std::unique_ptr<AstFunction> AstFunction::clone() const {
    return std::make_unique<AstFunction>(
        std::make_unique<AstPrototype>(
            Proto->getName(),
            Proto->getArgs()
        ),
        Body->clone()
    );
}

// Context class method implementations
InterpreterValue* Context::getValue(const std::string& name) const {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second.get();
    }
    return nullptr;
}

void Context::setValue(const std::string& name, std::unique_ptr<InterpreterValue> value) {
    variables[name] = std::move(value);
}

const AstFunction* Context::getFunction(const std::string& name) const {
    auto it = functions.find(name);
    if (it != functions.end()) {
        return it->second.get();
    }
    return nullptr;
}

void Context::addFunction(std::unique_ptr<AstFunction> func) {
    functions[func->getPrototype()->getName()] = std::move(func);
}

std::unique_ptr<Context> Context::cloneFunctionContext() const {
    auto funcContext = std::make_unique<Context>();
    for (const auto & [ key, value ] : functions) {
        funcContext->addFunction(std::unique_ptr<AstFunction>(value->clone()));
    }
    return funcContext;
}

std::unique_ptr<Context> Context::clone() const {
    auto newContext = std::make_unique<Context>();
    for (const auto & [ key, value ] : functions) {
        newContext->addFunction(std::unique_ptr<AstFunction>(value->clone()));
    }
    for (const auto& [name, val] : this->variables) {
        newContext->setValue(name, val->clone());
    }
    return newContext;
}

// AstExpr method implementations
std::unique_ptr<InterpreterValue> AstExprVariable::eval(const Context& context) const {
    auto value = context.getValue(Name);
    if (value) {
        return value->clone();
    }
    return nullptr;
}

std::unique_ptr<AstExpr> AstExprVariable::clone() const {
    return std::make_unique<AstExprVariable>(Name);
}

AstExprCall::AstExprCall(const std::string &Callee,
                         std::vector<std::unique_ptr<AstExpr>> Args)
    : Callee(Callee), Args(std::move(Args)) {}

std::unique_ptr<InterpreterValue> AstExprCall::eval(const Context& context) const {
    const AstFunction* calleeFunc = context.getFunction(Callee);
    if (!calleeFunc) {
        return nullptr;
    }

    std::vector<std::unique_ptr<InterpreterValue>> evaluatedArgs;
    for (const auto& arg : this->Args) {
        auto evaluated = arg->eval(context);
        if (!evaluated) {
            return nullptr;
        }
        evaluatedArgs.push_back(std::move(evaluated));
    }

    std::unique_ptr<Context> funcContext = context.cloneFunctionContext();
    const auto& protoArgs = calleeFunc->getPrototype()->getArgs();
    if (protoArgs.size() != evaluatedArgs.size()) {
        return nullptr;
    }
    for (size_t i = 0; i < protoArgs.size(); ++i) {
        funcContext->setValue(protoArgs[i], std::move(evaluatedArgs[i]));
    }

    return calleeFunc->eval(*funcContext);
}

std::unique_ptr<AstExpr> AstExprCall::clone() const {
    std::vector<std::unique_ptr<AstExpr>> clonedArgs;
    for (const auto& arg : Args) {
        clonedArgs.push_back(arg->clone());
    }
    return std::make_unique<AstExprCall>(Callee, std::move(clonedArgs));
}

AstExprLetIn::AstExprLetIn(const std::string &Variable,
                           std::unique_ptr<AstExpr> Expr,
                           std::unique_ptr<AstExpr> Body)
    : Variable(Variable), Expr(std::move(Expr)), Body(std::move(Body)) {}

std::unique_ptr<InterpreterValue> AstExprLetIn::eval(const Context& context) const {
    std::unique_ptr<InterpreterValue> evaluatedExpr = Expr->eval(context);
    if (!evaluatedExpr) {
        return nullptr;
    }

    std::unique_ptr<Context> newContext = context.clone();
    newContext->setValue(Variable, std::move(evaluatedExpr));

    return Body->eval(*newContext);
}

std::unique_ptr<AstExpr> AstExprLetIn::clone() const {
    return std::make_unique<AstExprLetIn>(Variable, Expr->clone(), Body->clone());
}

template <BinaryOpKindIntToInt OpKind>
AstExprBinaryIntToInt<OpKind>::AstExprBinaryIntToInt(std::unique_ptr<AstExpr> LHS, std::unique_ptr<AstExpr> RHS)
    : LHS(std::move(LHS)), RHS(std::move(RHS)) {}

template <BinaryOpKindIntToInt OpKind>
std::unique_ptr<InterpreterValue> AstExprBinaryIntToInt<OpKind>::eval(const Context& context) const {
    auto valueLHS = LHS->eval(context);
    auto valueRHS = RHS->eval(context);
    if (!valueLHS || !valueRHS) return nullptr;
    auto lhsLong = dynamic_cast<InterpreterValueLong*>(valueLHS.get());
    auto rhsLong = dynamic_cast<InterpreterValueLong*>(valueRHS.get());
    if (!lhsLong || !rhsLong) return nullptr;

    long lhsVal = lhsLong->getValue();
    long rhsVal = rhsLong->getValue();
    long result;

    if constexpr (OpKind == BinaryOpKindIntToInt::Add) {
        result = lhsVal + rhsVal;
    } else if constexpr (OpKind == BinaryOpKindIntToInt::Sub) {
        result = lhsVal - rhsVal;
    } else if constexpr (OpKind == BinaryOpKindIntToInt::Mul) {
        result = lhsVal * rhsVal;
    } else if constexpr (OpKind == BinaryOpKindIntToInt::Div) {
        if (rhsVal == 0) return nullptr;
        result = lhsVal / rhsVal;
    }
    return std::make_unique<InterpreterValueLong>(result);
}

template <BinaryOpKindIntToInt OpKind>
std::unique_ptr<AstExpr> AstExprBinaryIntToInt<OpKind>::clone() const {
    return std::make_unique<AstExprBinaryIntToInt<OpKind>>(LHS->clone(), RHS->clone());
}

template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>;
template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>;
template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>;
template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>;

template <BinaryOpKindIntToBool OpKind>
AstExprBinaryIntToBool<OpKind>::AstExprBinaryIntToBool(std::unique_ptr<AstExpr> LHS, std::unique_ptr<AstExpr> RHS)
    : LHS(std::move(LHS)), RHS(std::move(RHS)) {}

template <BinaryOpKindIntToBool OpKind>
std::unique_ptr<InterpreterValue> AstExprBinaryIntToBool<OpKind>::eval(const Context& context) const {
    auto valueLHS = LHS->eval(context);
    auto valueRHS = RHS->eval(context);
    if (!valueLHS || !valueRHS) return nullptr;
    auto lhsLong = dynamic_cast<InterpreterValueLong*>(valueLHS.get());
    auto rhsLong = dynamic_cast<InterpreterValueLong*>(valueRHS.get());
    if (!lhsLong || !rhsLong) return nullptr;

    long lhsVal = lhsLong->getValue();
    long rhsVal = rhsLong->getValue();
    bool result;
    if constexpr (OpKind == BinaryOpKindIntToBool::Eq) result = lhsVal == rhsVal;
    else if constexpr (OpKind == BinaryOpKindIntToBool::Neq) result = lhsVal != rhsVal;
    else if constexpr (OpKind == BinaryOpKindIntToBool::Leq) result = lhsVal <= rhsVal;
    else if constexpr (OpKind == BinaryOpKindIntToBool::Lt) result = lhsVal < rhsVal;
    else if constexpr (OpKind == BinaryOpKindIntToBool::Geq) result = lhsVal >= rhsVal;
    else if constexpr (OpKind == BinaryOpKindIntToBool::Gt) result = lhsVal > rhsVal;
    return std::make_unique<InterpreterValueBool>(result);
}

template <BinaryOpKindIntToBool OpKind>
std::unique_ptr<AstExpr> AstExprBinaryIntToBool<OpKind>::clone() const {
    return std::make_unique<AstExprBinaryIntToBool<OpKind>>(LHS->clone(), RHS->clone());
}

template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>;

template <BinaryOpKindBoolToBool OpKind>
AstExprBinaryBoolToBool<OpKind>::AstExprBinaryBoolToBool(std::unique_ptr<AstExpr> LHS, std::unique_ptr<AstExpr> RHS)
    : LHS(std::move(LHS)), RHS(std::move(RHS)) {}

template <BinaryOpKindBoolToBool OpKind>
std::unique_ptr<InterpreterValue> AstExprBinaryBoolToBool<OpKind>::eval(const Context& context) const {
    auto valueLHS = LHS->eval(context);
    auto valueRHS = RHS->eval(context);
    if (!valueLHS || !valueRHS) return nullptr;
    auto lhsBool = dynamic_cast<InterpreterValueBool*>(valueLHS.get());
    auto rhsBool = dynamic_cast<InterpreterValueBool*>(valueRHS.get());
    if (!lhsBool || !rhsBool) return nullptr;

    bool lhsVal = lhsBool->getValue();
    bool rhsVal = rhsBool->getValue();
    bool result;
    if constexpr (OpKind == BinaryOpKindBoolToBool::And) result = lhsVal && rhsVal;
    else if constexpr (OpKind == BinaryOpKindBoolToBool::Or) result = lhsVal || rhsVal;
    return std::make_unique<InterpreterValueBool>(result);
}

template <BinaryOpKindBoolToBool OpKind>
std::unique_ptr<AstExpr> AstExprBinaryBoolToBool<OpKind>::clone() const {
    return std::make_unique<AstExprBinaryBoolToBool<OpKind>>(LHS->clone(), RHS->clone());
}

template class AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>;
template class AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>;

AstExprMatchPath::AstExprMatchPath(std::unique_ptr<AstExpr> guard, std::unique_ptr<AstExpr> body)
    : Guard(std::move(guard)), Body(std::move(body)) {}

std::unique_ptr<AstExprMatchPath> AstExprMatchPath::clone() const {
    return std::make_unique<AstExprMatchPath>(Guard->clone(), Body->clone());
}

AstExprMatch::AstExprMatch(std::vector<std::unique_ptr<AstExprMatchPath>> Paths)
    : Paths(std::move(Paths)) {}

std::unique_ptr<InterpreterValue> AstExprMatch::eval(const Context& context) const {
    for (const auto& path : this->Paths) {
        auto evaluated = path->Guard->eval(context);
        if (!evaluated) {
            return nullptr;
        }
        auto evaluatedBool = dynamic_cast<InterpreterValueBool*>(evaluated.get());
        if (!evaluatedBool) {
            return nullptr;
        }
        if (evaluatedBool->getValue()) {
            return path->Body->eval(context);
        }
    }
    return nullptr;
}

std::unique_ptr<AstExpr> AstExprMatch::clone() const {
    std::vector<std::unique_ptr<AstExprMatchPath>> clonedPaths;
    for (const auto& path : Paths) {
        clonedPaths.push_back(path->clone());
    }
    return std::make_unique<AstExprMatch>(std::move(clonedPaths));
}
