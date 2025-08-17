#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>


class InterpreterValue {
public:
    virtual ~InterpreterValue() = default;
    virtual std::unique_ptr<InterpreterValue> clone() const = 0;
};

class InterpreterValueLong : public InterpreterValue {
    long Value;
public:
    InterpreterValueLong(const long &Value) : Value(Value) {}
    long getValue() const { return Value; }
    std::unique_ptr<InterpreterValue> clone() const override {
        return std::make_unique<InterpreterValueLong>(Value);
    }
};

class InterpreterValueBool : public InterpreterValue {
    bool Value;
public:
    InterpreterValueBool(const bool &Value) : Value(Value) {}
    long getValue() const { return Value; }
    std::unique_ptr<InterpreterValue> clone() const override {
        return std::make_unique<InterpreterValueBool>(Value);
    }
};



// Forward declarations
class Context;
class AstExpr;
class AstFunction;
class AstPrototype;


class AstExpr {
public:
    virtual ~AstExpr() = default;
    virtual std::unique_ptr<InterpreterValue> eval(const Context& context) const = 0;
    virtual std::unique_ptr<AstExpr> clone() const = 0;
};

class AstExprConst : public AstExpr {
    virtual std::unique_ptr<InterpreterValue> getValue() const = 0;
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override {
        (void) context; // silence unused parameter
        return getValue();
    }
};

class AstExprConstLong : public AstExprConst {
    long Value;
public:
    AstExprConstLong(const long &Value) : Value(Value) {}
    std::unique_ptr<InterpreterValue> getValue() const override {
        return std::make_unique<InterpreterValueLong>(Value);
    }
    std::unique_ptr<AstExpr> clone() const override {
        return std::make_unique<AstExprConstLong>(Value);
    }
};

class AstExprConstBool : public AstExprConst {
    bool Value;
public:
    AstExprConstBool(const bool &Value) : Value(Value) {}
    std::unique_ptr<InterpreterValue> getValue() const override {
        return std::make_unique<InterpreterValueBool>(Value);
    }
    std::unique_ptr<AstExpr> clone() const override {
        return std::make_unique<AstExprConstBool>(Value);
    }
};

class AstPrototype {
    std::string Name;
    std::vector<std::string> Args;
public:
    AstPrototype(const std::string &Name, std::vector<std::string> Args)
        : Name(Name), Args(std::move(Args)) {}
    const std::string &getName() const { return Name; }
    const std::vector<std::string> &getArgs() const { return Args; }
};

class AstFunction {
    std::unique_ptr<AstPrototype> Proto;
    std::unique_ptr<AstExpr> Body;
public:
    AstFunction(std::unique_ptr<AstPrototype> Proto, std::unique_ptr<AstExpr> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}

    const AstPrototype* getPrototype() const { return Proto.get(); }
    std::unique_ptr<InterpreterValue> eval(const Context& context) const;
    std::unique_ptr<AstFunction> clone() const {
        return std::make_unique<AstFunction>(
            std::make_unique<AstPrototype>(
                Proto->getName(),
                Proto->getArgs()
            ),
            Body->clone()
        );
    }
};

class Context {
public:
    InterpreterValue* getValue(const std::string& name) const {
        auto it = variables.find(name);
        if (it != variables.end()) {
            return it->second.get();
        }
        return nullptr;
    }
    void setValue(const std::string& name, std::unique_ptr<InterpreterValue> value) {
        variables[name] = std::move(value);
    }
    const AstFunction* getFunction(const std::string& name) const {
        auto it = functions.find(name);
        if (it != functions.end()) {
            return it->second.get();
        }
        return nullptr;
    }
    void addFunction(std::unique_ptr<AstFunction> func) {
        functions[func->getPrototype()->getName()] = std::move(func);
    }
    std::unique_ptr<Context> cloneFunctionContext() const {
        auto funcContext = std::make_unique<Context>();
        for (const auto & [ key, value ] : functions) {
            funcContext->addFunction(std::unique_ptr<AstFunction>(value->clone()));
        }
        return funcContext;
    }
    std::unique_ptr<Context> clone() const {
        auto newContext = std::make_unique<Context>();
        for (const auto & [ key, value ] : functions) {
            newContext->addFunction(std::unique_ptr<AstFunction>(value->clone()));
        }
        for (const auto& [name, val] : this->variables) {
            newContext->setValue(name, val->clone());
        }
        return newContext;
    }
private:
    std::unordered_map<std::string, std::unique_ptr<InterpreterValue>> variables;
    std::unordered_map<std::string, std::unique_ptr<AstFunction>> functions;
};


class AstExprVariable : public AstExpr {
    std::string Name;
public:
    AstExprVariable(const std::string &Name) : Name(Name) {}
    const std::string& getName() const { return Name; }
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override {
        auto value = context.getValue(Name);
        if (value) {
            return value->clone();
        }
        return nullptr;
    }
    std::unique_ptr<AstExpr> clone() const override {
        return std::make_unique<AstExprVariable>(Name);
    }
};

class AstExprCall : public AstExpr {
    std::string Callee;
    std::vector<std::unique_ptr<AstExpr>> Args;
public:
    AstExprCall(const std::string &Callee,
                std::vector<std::unique_ptr<AstExpr>> Args)
        : Callee(Callee), Args(std::move(Args)) {}
    
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override {
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
    
    std::unique_ptr<AstExpr> clone() const override {
        std::vector<std::unique_ptr<AstExpr>> clonedArgs;
        for (const auto& arg : Args) {
            clonedArgs.push_back(arg->clone());
        }
        return std::make_unique<AstExprCall>(Callee, std::move(clonedArgs));
    }
};

class AstExprLetIn : public AstExpr {
    std::string Variable;
    std::unique_ptr<AstExpr> Expr;
    std::unique_ptr<AstExpr> Body;
public:
    AstExprLetIn(const std::string &Variable,
                std::unique_ptr<AstExpr> Expr,
                std::unique_ptr<AstExpr> Body)
        : Variable(Variable), Expr(std::move(Expr)), Body(std::move(Body)) {}
    
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override {

       std::unique_ptr<InterpreterValue> evaluatedExpr = Expr->eval(context);
        if (!evaluatedExpr) {
            return nullptr;
        }

        std::unique_ptr<Context> newContext = context.clone();
        newContext->setValue(Variable, std::move(evaluatedExpr));
        
        return Body->eval(*newContext);
    }
    
    std::unique_ptr<AstExpr> clone() const override {
        return std::make_unique<AstExprLetIn>(Variable, Expr->clone(), Body->clone());
    }
};

enum class BinaryOpKindIntToInt {
    Add, Sub, Mul, Div,
};

template <BinaryOpKindIntToInt OpKind>
class AstExprBinaryIntToInt : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinaryIntToInt(std::unique_ptr<AstExpr> LHS, std::unique_ptr<AstExpr> RHS)
        : LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override {
        auto valueLHS = LHS->eval(context);
        auto valueRHS = RHS->eval(context);

        if (!valueLHS || !valueRHS) {
            return nullptr;
        }

        auto lhsLong = dynamic_cast<InterpreterValueLong*>(valueLHS.get());
        auto rhsLong = dynamic_cast<InterpreterValueLong*>(valueRHS.get());
        if (!lhsLong || !rhsLong) {
            return nullptr; // type mismatch
        }

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
            if (rhsVal == 0) {
                return nullptr; // division by zero
            }
            result = lhsVal / rhsVal;
        }

        return std::make_unique<InterpreterValueLong>(result);
    }

    std::unique_ptr<AstExpr> clone() const override {
        return std::make_unique<AstExprBinaryIntToInt<OpKind>>(
            LHS->clone(),
            RHS->clone()
        );
    }
};

enum class BinaryOpKindIntToBool {
    Eq, Neq, Leq, Lt, Geq, Gt
};

template <BinaryOpKindIntToBool OpKind>
class AstExprBinaryIntToBool : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinaryIntToBool(std::unique_ptr<AstExpr> LHS, std::unique_ptr<AstExpr> RHS)
        : LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override {
        auto valueLHS = LHS->eval(context);
        auto valueRHS = RHS->eval(context);

        if (!valueLHS || !valueRHS) {
            return nullptr;
        }

        auto lhsLong = dynamic_cast<InterpreterValueLong*>(valueLHS.get());
        auto rhsLong = dynamic_cast<InterpreterValueLong*>(valueRHS.get());
        if (!lhsLong || !rhsLong) {
            return nullptr; // type mismatch
        }

        long lhsVal = lhsLong->getValue();
        long rhsVal = rhsLong->getValue();
        
        
        bool result;

        if constexpr (OpKind == BinaryOpKindIntToBool::Eq) {
            result = lhsVal == rhsVal;
        } else if constexpr (OpKind == BinaryOpKindIntToBool::Neq) {
            result = lhsVal != rhsVal;
        } else if constexpr (OpKind == BinaryOpKindIntToBool::Leq) {
            result = lhsVal <= rhsVal;
        } else if constexpr (OpKind == BinaryOpKindIntToBool::Lt) {
            result = lhsVal < rhsVal;
        } else if constexpr (OpKind == BinaryOpKindIntToBool::Geq) {
            result = lhsVal >= rhsVal;
        } else if constexpr (OpKind == BinaryOpKindIntToBool::Gt) {
            result = lhsVal > rhsVal;
        }

        return std::make_unique<InterpreterValueBool>(result);
    }

    std::unique_ptr<AstExpr> clone() const override {
        return std::make_unique<AstExprBinaryIntToBool<OpKind>>(
            LHS->clone(),
            RHS->clone()
        );
    }
};

enum class BinaryOpKindBoolToBool {
    And, Or
};

template <BinaryOpKindBoolToBool OpKind>
class AstExprBinaryBoolToBool : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinaryBoolToBool(std::unique_ptr<AstExpr> LHS, std::unique_ptr<AstExpr> RHS)
        : LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    std::unique_ptr<InterpreterValue> eval(const Context& context) const override {
        auto valueLHS = LHS->eval(context);
        auto valueRHS = RHS->eval(context);

        if (!valueLHS || !valueRHS) {
            return nullptr;
        }

        auto lhsBool = dynamic_cast<InterpreterValueBool*>(valueLHS.get());
        auto rhsBool = dynamic_cast<InterpreterValueBool*>(valueRHS.get());
        if (!lhsBool || !rhsBool) {
            return nullptr; // type mismatch
        }

        bool lhsVal = lhsBool->getValue();
        bool rhsVal = rhsBool->getValue();
        bool result;

        if constexpr (OpKind == BinaryOpKindBoolToBool::And) {
            result = lhsVal && rhsVal;
        } else if constexpr (OpKind == BinaryOpKindBoolToBool::Or) {
            result = lhsVal || rhsVal;
        }

        return std::make_unique<InterpreterValueBool>(result);
    }

    std::unique_ptr<AstExpr> clone() const override {
        return std::make_unique<AstExprBinaryBoolToBool<OpKind>>(
            LHS->clone(),
            RHS->clone()
        );
    }
};

std::unique_ptr<InterpreterValue> AstFunction::eval(const Context& context) const {
    return Body->eval(context);
}