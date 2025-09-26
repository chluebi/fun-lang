#include "interpreter.hpp"
#include "interpreter_exception.hpp"
#include <utility>
#include <string>

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

std::unique_ptr<InterpreterValue> Interpreter::runWithContext(const AstExpr& expr, const Context& newContext) const {
    // Cast away constness to modify the internal context pointer,
    // as it's a private mutable state of the interpreter for threading the context.
    const_cast<Interpreter*>(this)->CurrentContext = &newContext;
    auto result = expr.accept(const_cast<Interpreter&>(*this));
    return result;
}

std::unique_ptr<InterpreterValue> Interpreter::eval(const AstExpr& expr, const Context& context) const {
    return runWithContext(expr, context);
}

std::unique_ptr<InterpreterValue> Interpreter::visit(const AstExprConstLong& expr) {
    return std::make_unique<InterpreterValueLong>(expr.getValue());
}

std::unique_ptr<InterpreterValue> Interpreter::visit(const AstExprConstBool& expr) {
    return std::make_unique<InterpreterValueBool>(expr.getValue());
}

std::unique_ptr<InterpreterValue> Interpreter::visit(const AstExprVariable& expr) {
    auto value = CurrentContext->getValue(expr.getName());
    if (!value) {
        throw UndefinedVariableException(expr.getName(), expr.getLocation());
    }
    return value->clone();
}

std::unique_ptr<InterpreterValue> Interpreter::visit(const AstExprCall& expr) {
    const AstFunction* calleeFunc = CurrentContext->getFunction(expr.getCallee());
    if (!calleeFunc) {
        throw UndefinedFunctionException(expr.getCallee(), expr.getLocation());
    }
    std::vector<std::unique_ptr<InterpreterValue>> evaluatedArgs;
    for (const auto& arg : expr.getArgs()) {
        auto evaluated = this->eval(*arg, *CurrentContext);
        evaluatedArgs.push_back(std::move(evaluated));
    }
    std::unique_ptr<Context> funcContext = CurrentContext->cloneFunctionContext();
    const auto& protoArgs = calleeFunc->getPrototype()->getArgs();
    if (protoArgs.size() != evaluatedArgs.size()) {
        throw ArityMismatchException(calleeFunc->getPrototype()->getName(), protoArgs.size(), evaluatedArgs.size(), expr.getLocation());
    }
    for (size_t i = 0; i < protoArgs.size(); ++i) {
        funcContext->setValue(protoArgs[i].Name, std::move(evaluatedArgs[i]));
    }
    return runWithContext(*calleeFunc->getBody(), *funcContext);
}

std::unique_ptr<InterpreterValue> Interpreter::visit(const AstExprLetIn& expr) {
    std::unique_ptr<InterpreterValue> evaluatedExpr = this->eval(*expr.getExpr(), *CurrentContext);
    
    std::unique_ptr<Context> newContext = CurrentContext->clone();
    newContext->setValue(expr.getVariable(), std::move(evaluatedExpr));
    
    return runWithContext(*expr.getBody(), *newContext);
}

std::unique_ptr<InterpreterValue> Interpreter::visit(const AstExprMatch& expr) {
    for (const auto& path : expr.getPaths()) {
        auto evaluated = this->eval(*path->getGuard(), *CurrentContext);
        
        auto evaluatedBool = dynamic_cast<InterpreterValueBool*>(evaluated.get());
        if (!evaluatedBool) {
            throw TypeMismatchException("Match guard must evaluate to a boolean", path->getLocation());
        }
        if (evaluatedBool->getValue()) {
            return this->eval(*path->getBody(), *CurrentContext);
        }
    }
    throw NoMatchFoundException(expr.getLocation());
}

#define IMPLEMENT_BIN_INT_TO_INT_VISIT(OP_KIND) \
    std::unique_ptr<InterpreterValue> Interpreter::visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::OP_KIND>& expr) { \
        return evalBinaryIntToInt(*expr.getLHS(), *expr.getRHS(), BinaryOpKindIntToInt::OP_KIND); \
    }

IMPLEMENT_BIN_INT_TO_INT_VISIT(Add)
IMPLEMENT_BIN_INT_TO_INT_VISIT(Sub)
IMPLEMENT_BIN_INT_TO_INT_VISIT(Mul)
IMPLEMENT_BIN_INT_TO_INT_VISIT(Div)

#undef IMPLEMENT_BIN_INT_TO_INT_VISIT

#define IMPLEMENT_BIN_INT_TO_BOOL_VISIT(OP_KIND) \
    std::unique_ptr<InterpreterValue> Interpreter::visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::OP_KIND>& expr) { \
        return evalBinaryIntToBool(*expr.getLHS(), *expr.getRHS(), BinaryOpKindIntToBool::OP_KIND); \
    }

IMPLEMENT_BIN_INT_TO_BOOL_VISIT(Eq)
IMPLEMENT_BIN_INT_TO_BOOL_VISIT(Neq)
IMPLEMENT_BIN_INT_TO_BOOL_VISIT(Leq)
IMPLEMENT_BIN_INT_TO_BOOL_VISIT(Lt)
IMPLEMENT_BIN_INT_TO_BOOL_VISIT(Geq)
IMPLEMENT_BIN_INT_TO_BOOL_VISIT(Gt)

#undef IMPLEMENT_BIN_INT_TO_BOOL_VISIT

#define IMPLEMENT_BIN_BOOL_TO_BOOL_VISIT(OP_KIND) \
    std::unique_ptr<InterpreterValue> Interpreter::visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::OP_KIND>& expr) { \
        return evalBinaryBoolToBool(*expr.getLHS(), *expr.getRHS(), BinaryOpKindBoolToBool::OP_KIND); \
    }

IMPLEMENT_BIN_BOOL_TO_BOOL_VISIT(And)
IMPLEMENT_BIN_BOOL_TO_BOOL_VISIT(Or)

#undef IMPLEMENT_BIN_BOOL_TO_BOOL_VISIT



std::unique_ptr<InterpreterValue> Interpreter::evalBinaryIntToInt(
    const AstExpr& lhs, const AstExpr& rhs, BinaryOpKindIntToInt op) const
{
    // eval arguments using CurrentContext
    auto valueLHS = this->eval(lhs, *CurrentContext);
    auto valueRHS = this->eval(rhs, *CurrentContext);
    
    auto lhsLong = dynamic_cast<InterpreterValueLong*>(valueLHS.get());
    auto rhsLong = dynamic_cast<InterpreterValueLong*>(valueRHS.get());
    
    if (!lhsLong) {
        throw TypeMismatchException("LHS of integer binary operation is not an integer", lhs.getLocation());
    }
    if (!rhsLong) {
        throw TypeMismatchException("RHS of integer binary operation is not an integer", rhs.getLocation());
    }

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
        if (rhsVal == 0) {
            throw DivisionByZeroException(rhs.getLocation());
        }
        result = lhsVal / rhsVal;
    } else {
        throw InterpreterException("Invalid IntToInt operation kind", lhs.getLocation());
    }
    return std::make_unique<InterpreterValueLong>(result);
}

std::unique_ptr<InterpreterValue> Interpreter::evalBinaryIntToBool(
    const AstExpr& lhs, const AstExpr& rhs, BinaryOpKindIntToBool op) const
{
    auto valueLHS = this->eval(lhs, *CurrentContext);
    auto valueRHS = this->eval(rhs, *CurrentContext);
    
    auto lhsLong = dynamic_cast<InterpreterValueLong*>(valueLHS.get());
    auto rhsLong = dynamic_cast<InterpreterValueLong*>(valueRHS.get());
    
    if (!lhsLong) {
        throw TypeMismatchException("LHS of integer comparison is not an integer", lhs.getLocation());
    }
    if (!rhsLong) {
        throw TypeMismatchException("RHS of integer comparison is not an integer", rhs.getLocation());
    }

    long lhsVal = lhsLong->getValue();
    long rhsVal = rhsLong->getValue();
    bool result;

    if (op == BinaryOpKindIntToBool::Eq) result = lhsVal == rhsVal;
    else if (op == BinaryOpKindIntToBool::Neq) result = lhsVal != rhsVal;
    else if (op == BinaryOpKindIntToBool::Leq) result = lhsVal <= rhsVal;
    else if (op == BinaryOpKindIntToBool::Lt) result = lhsVal < rhsVal;
    else if (op == BinaryOpKindIntToBool::Geq) result = lhsVal >= rhsVal;
    else if (op == BinaryOpKindIntToBool::Gt) result = lhsVal > rhsVal;
    else throw InterpreterException("Invalid IntToBool operation kind", lhs.getLocation());

    return std::make_unique<InterpreterValueBool>(result);
}

std::unique_ptr<InterpreterValue> Interpreter::evalBinaryBoolToBool(
    const AstExpr& lhs, const AstExpr& rhs, BinaryOpKindBoolToBool op) const
{
    auto valueLHS = this->eval(lhs, *CurrentContext);
    auto valueRHS = this->eval(rhs, *CurrentContext);

    auto lhsBool = dynamic_cast<InterpreterValueBool*>(valueLHS.get());
    auto rhsBool = dynamic_cast<InterpreterValueBool*>(valueRHS.get());
    
    if (!lhsBool) {
        throw TypeMismatchException("LHS of boolean binary operation is not a boolean", lhs.getLocation());
    }
    if (!rhsBool) {
        throw TypeMismatchException("RHS of boolean binary operation is not a boolean", rhs.getLocation());
    }

    bool lhsVal = lhsBool->getValue();
    bool rhsVal = rhsBool->getValue();
    bool result;
    if (op == BinaryOpKindBoolToBool::And) result = lhsVal && rhsVal;
    else if (op == BinaryOpKindBoolToBool::Or) result = lhsVal || rhsVal;
    else throw InterpreterException("Invalid BoolToBool operation kind", lhs.getLocation());

    return std::make_unique<InterpreterValueBool>(result);
}