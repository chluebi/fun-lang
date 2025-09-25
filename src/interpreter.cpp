#include "interpreter.hpp"
#include <utility>

InterpreterValueLong::InterpreterValueLong(const long &Value) : Value(Value) {}
long InterpreterValueLong::getValue() const { return Value; }
std::unique_ptr<InterpreterValue> InterpreterValueLong::clone() const {
    return std::make_unique<InterpreterValueLong>(Value);
}

InterpreterValueBool::InterpreterValueBool(const bool &Value) : Value(Value) {}
bool InterpreterValueBool::getValue() const { return Value; }
std::unique_ptr<InterpreterValue> InterpreterValueBool::clone() const {
    return std::make_unique<InterpreterValueBool>(Value);
}

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

std::unique_ptr<InterpreterValue> Interpreter::eval(const AstExpr& expr, const Context& context) const {
    if (const auto* constExpr = dynamic_cast<const AstExprConstLong*>(&expr)) {
        return std::make_unique<InterpreterValueLong>(constExpr->getValue());
    }
    if (const auto* constExpr = dynamic_cast<const AstExprConstBool*>(&expr)) {
        return std::make_unique<InterpreterValueBool>(constExpr->getValue());
    }
    if (const auto* varExpr = dynamic_cast<const AstExprVariable*>(&expr)) {
        auto value = context.getValue(varExpr->getName());
        return value ? value->clone() : nullptr;
    }
    if (const auto* callExpr = dynamic_cast<const AstExprCall*>(&expr)) {
        const AstFunction* calleeFunc = context.getFunction(callExpr->getCallee());
        if (!calleeFunc) {
            return nullptr;
        }
        std::vector<std::unique_ptr<InterpreterValue>> evaluatedArgs;
        for (const auto& arg : callExpr->getArgs()) {
            auto evaluated = this->eval(*arg, context);
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
            funcContext->setValue(protoArgs[i].Name, std::move(evaluatedArgs[i]));
        }
        return this->eval(*calleeFunc->getBody(), *funcContext);
    }
    if (const auto* letInExpr = dynamic_cast<const AstExprLetIn*>(&expr)) {
        std::unique_ptr<InterpreterValue> evaluatedExpr = this->eval(*letInExpr->getExpr(), context);
        if (!evaluatedExpr) {
            return nullptr;
        }
        std::unique_ptr<Context> newContext = context.clone();
        newContext->setValue(letInExpr->getVariable(), std::move(evaluatedExpr));
        return this->eval(*letInExpr->getBody(), *newContext);
    }
    if (const auto* matchExpr = dynamic_cast<const AstExprMatch*>(&expr)) {
        for (const auto& path : matchExpr->getPaths()) {
            auto evaluated = this->eval(*path->getGuard(), context);
            if (!evaluated) {
                return nullptr;
            }
            auto evaluatedBool = dynamic_cast<InterpreterValueBool*>(evaluated.get());
            if (!evaluatedBool) {
                return nullptr;
            }
            if (evaluatedBool->getValue()) {
                return this->eval(*path->getBody(), context);
            }
        }
        return nullptr;
    }

#define EVAL_BIN_INT_TO_INT(OP_TYPE, OP) \
    if (const auto* binExpr = dynamic_cast<const AstExprBinaryIntToInt<OP_TYPE>*>(&expr)) { \
        return evalBinaryIntToInt(*binExpr->getLHS(), *binExpr->getRHS(), context, OP_TYPE); \
    }

    EVAL_BIN_INT_TO_INT(BinaryOpKindIntToInt::Add, +);
    EVAL_BIN_INT_TO_INT(BinaryOpKindIntToInt::Sub, -);
    EVAL_BIN_INT_TO_INT(BinaryOpKindIntToInt::Mul, *);
    EVAL_BIN_INT_TO_INT(BinaryOpKindIntToInt::Div, /);

#undef EVAL_BIN_INT_TO_INT

#define EVAL_BIN_INT_TO_BOOL(OP_TYPE, OP) \
    if (const auto* binExpr = dynamic_cast<const AstExprBinaryIntToBool<OP_TYPE>*>(&expr)) { \
        return evalBinaryIntToBool(*binExpr->getLHS(), *binExpr->getRHS(), context, OP_TYPE); \
    }
    EVAL_BIN_INT_TO_BOOL(BinaryOpKindIntToBool::Eq, ==);
    EVAL_BIN_INT_TO_BOOL(BinaryOpKindIntToBool::Neq, !=);
    EVAL_BIN_INT_TO_BOOL(BinaryOpKindIntToBool::Leq, <=);
    EVAL_BIN_INT_TO_BOOL(BinaryOpKindIntToBool::Lt, <);
    EVAL_BIN_INT_TO_BOOL(BinaryOpKindIntToBool::Geq, >=);
    EVAL_BIN_INT_TO_BOOL(BinaryOpKindIntToBool::Gt, >);

#undef EVAL_BIN_INT_TO_BOOL

#define EVAL_BIN_BOOL_TO_BOOL(OP_TYPE, OP) \
    if (const auto* binExpr = dynamic_cast<const AstExprBinaryBoolToBool<OP_TYPE>*>(&expr)) { \
        return evalBinaryBoolToBool(*binExpr->getLHS(), *binExpr->getRHS(), context, OP_TYPE); \
    }
    EVAL_BIN_BOOL_TO_BOOL(BinaryOpKindBoolToBool::And, &&);
    EVAL_BIN_BOOL_TO_BOOL(BinaryOpKindBoolToBool::Or, ||);

#undef EVAL_BIN_BOOL_TO_BOOL

    return nullptr;
}

std::unique_ptr<InterpreterValue> Interpreter::evalBinaryIntToInt(
    const AstExpr& lhs, const AstExpr& rhs, const Context& context,
    BinaryOpKindIntToInt op) const
{
    auto valueLHS = this->eval(lhs, context);
    auto valueRHS = this->eval(rhs, context);
    if (!valueLHS || !valueRHS) return nullptr;
    auto lhsLong = dynamic_cast<InterpreterValueLong*>(valueLHS.get());
    auto rhsLong = dynamic_cast<InterpreterValueLong*>(valueRHS.get());
    if (!lhsLong || !rhsLong) return nullptr;

    long lhsVal = lhsLong->getValue();
    long rhsVal = rhsLong->getValue();
    long result;

    if (op == BinaryOpKindIntToInt::Add) {
        result = lhsVal + rhsVal;
    } else if (op == BinaryOpKindIntToInt::Sub) {
        result = lhsVal - rhsVal;
    } else if (op == BinaryOpKindIntToInt::Mul) {
        result = lhsVal * rhsVal;
    } else if (op == BinaryOpKindIntToInt::Div) {
        if (rhsVal == 0) return nullptr;
        result = lhsVal / rhsVal;
    } else {
        return nullptr;
    }
    return std::make_unique<InterpreterValueLong>(result);
}

std::unique_ptr<InterpreterValue> Interpreter::evalBinaryIntToBool(
    const AstExpr& lhs, const AstExpr& rhs, const Context& context,
    BinaryOpKindIntToBool op) const
{
    auto valueLHS = this->eval(lhs, context);
    auto valueRHS = this->eval(rhs, context);
    if (!valueLHS || !valueRHS) return nullptr;
    auto lhsLong = dynamic_cast<InterpreterValueLong*>(valueLHS.get());
    auto rhsLong = dynamic_cast<InterpreterValueLong*>(valueRHS.get());
    if (!lhsLong || !rhsLong) return nullptr;

    long lhsVal = lhsLong->getValue();
    long rhsVal = rhsLong->getValue();
    bool result;

    if (op == BinaryOpKindIntToBool::Eq) result = lhsVal == rhsVal;
    else if (op == BinaryOpKindIntToBool::Neq) result = lhsVal != rhsVal;
    else if (op == BinaryOpKindIntToBool::Leq) result = lhsVal <= rhsVal;
    else if (op == BinaryOpKindIntToBool::Lt) result = lhsVal < rhsVal;
    else if (op == BinaryOpKindIntToBool::Geq) result = lhsVal >= rhsVal;
    else if (op == BinaryOpKindIntToBool::Gt) result = lhsVal > rhsVal;
    else return nullptr;

    return std::make_unique<InterpreterValueBool>(result);
}

std::unique_ptr<InterpreterValue> Interpreter::evalBinaryBoolToBool(
    const AstExpr& lhs, const AstExpr& rhs, const Context& context,
    BinaryOpKindBoolToBool op) const
{
    auto valueLHS = this->eval(lhs, context);
    auto valueRHS = this->eval(rhs, context);
    if (!valueLHS || !valueRHS) return nullptr;
    auto lhsBool = dynamic_cast<InterpreterValueBool*>(valueLHS.get());
    auto rhsBool = dynamic_cast<InterpreterValueBool*>(valueRHS.get());
    if (!lhsBool || !rhsBool) return nullptr;

    bool lhsVal = lhsBool->getValue();
    bool rhsVal = rhsBool->getValue();
    bool result;
    if (op == BinaryOpKindBoolToBool::And) result = lhsVal && rhsVal;
    else if (op == BinaryOpKindBoolToBool::Or) result = lhsVal || rhsVal;
    else return nullptr;

    return std::make_unique<InterpreterValueBool>(result);
}