#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>

#include "source_location.hpp"

// Forward declarations to resolve circular dependencies
class Context;
class AstExpr;
class AstFunction;
class AstPrototype;

// Base class for all runtime values
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
    
    std::unique_ptr<InterpreterValue> clone() const {
        return std::make_unique<InterpreterValueLong>(Value);
    }
};

class InterpreterValueBool : public InterpreterValue {
    bool Value;
public:
    InterpreterValueBool(const bool &Value) : Value(Value) {}
    bool getValue() const { return Value; }

    std::unique_ptr<InterpreterValue> clone() const {
        return std::make_unique<InterpreterValueBool>(Value);
    }
};

// Base class for all AST nodes
class AstExpr {
protected:
    SourceLocation Location;
public:
    AstExpr(const SourceLocation& loc) : Location(loc) {}

    virtual ~AstExpr() = default;
    virtual std::unique_ptr<InterpreterValue> eval(const Context& context) const = 0;
    virtual std::unique_ptr<AstExpr> clone() const = 0;
    const SourceLocation& getLocation() const { return Location; }
};

// Constant expressions
class AstExprConst : public AstExpr {
protected:
    virtual std::unique_ptr<InterpreterValue> getValue() const = 0;
public:
    AstExprConst(const SourceLocation& loc) : AstExpr(loc) {}

    std::unique_ptr<InterpreterValue> eval(const Context& context) const {
        (void) context; // silence unused parameter
        return getValue();
    }
};

class AstExprConstLong : public AstExprConst {
    long Value;
public:
    AstExprConstLong(const SourceLocation &loc, const long &Value) : AstExprConst(loc), Value(Value) {}
    
    std::unique_ptr<InterpreterValue> getValue() const {
        return std::make_unique<InterpreterValueLong>(Value);
    }

    std::unique_ptr<AstExpr> clone() const {
        return std::make_unique<AstExprConstLong>(Location, Value);
    }

};

class AstExprConstBool : public AstExprConst {
    bool Value;
public:
    AstExprConstBool(const SourceLocation &loc, const bool &Value) : AstExprConst(loc), Value(Value) {}

    std::unique_ptr<InterpreterValue> getValue() const {
        return std::make_unique<InterpreterValueBool>(Value);
    }

    std::unique_ptr<AstExpr> clone() const {
        return std::make_unique<AstExprConstBool>(Location, Value);
    }
};

struct AstArg {
    SourceLocation Location;
    std::string Name;

    AstArg(const SourceLocation& loc, const std::string& name)
        : Location(loc), Name(name) {}
};

// Function prototype (name and arguments)
class AstPrototype {
    SourceLocation Location;
    std::string Name;
    std::vector<AstArg> Args;
public:
    AstPrototype(const SourceLocation& loc, const std::string& name, std::vector<AstArg> args)
        : Location(loc), Name(name), Args(std::move(args)) {}

    const SourceLocation& getLocation() const { return Location; }
    const std::string& getName() const { return Name; }
    const std::vector<AstArg>& getArgs() const { return Args; }
};

// Function definition
class AstFunction {
    SourceLocation Location;
    std::unique_ptr<AstPrototype> Proto;
    std::unique_ptr<AstExpr> Body;
public:
    AstFunction(const SourceLocation &loc, std::unique_ptr<AstPrototype> Proto, std::unique_ptr<AstExpr> Body)
        : Location(loc), Proto(std::move(Proto)), Body(std::move(Body)) {}
    const SourceLocation& getLocation() const { return Location; }
    const AstPrototype* getPrototype() const { return Proto.get(); }
    
    std::unique_ptr<InterpreterValue> eval(const Context& context) const {
        return Body->eval(context);
    }

    std::unique_ptr<AstFunction> clone() const {
        std::vector<AstArg> clonedArgs;
        for (const auto& arg : Proto->getArgs()) {
            clonedArgs.push_back(AstArg(arg.Location, arg.Name));
        }

        return std::make_unique<AstFunction>(
            Location,
            std::make_unique<AstPrototype>(
                Proto->getLocation(),
                Proto->getName(),
                std::move(clonedArgs)
            ),
            Body->clone()
        );
    }
};

// Execution context for variables and functions
class Context {
private:
    std::unordered_map<std::string, std::unique_ptr<InterpreterValue>> variables;
    std::unordered_map<std::string, std::unique_ptr<AstFunction>> functions;
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
};

// AST nodes for various expressions
class AstExprVariable : public AstExpr {
    std::string Name;
public:
    AstExprVariable(const SourceLocation &loc, const std::string &Name) : AstExpr(loc), Name(Name) {}
    const std::string& getName() const { return Name; }

    std::unique_ptr<InterpreterValue> eval(const Context& context) const {
        auto value = context.getValue(Name);
        if (value) {
            return value->clone();
        }
        return nullptr;
    }

    std::unique_ptr<AstExpr> clone() const {
        return std::make_unique<AstExprVariable>(Location, Name);
    }
};


class AstExprCall : public AstExpr {
    std::string Callee;
    std::vector<std::unique_ptr<AstExpr>> Args;
public:
    AstExprCall(const SourceLocation &loc, const std::string &Callee,
                std::vector<std::unique_ptr<AstExpr>> Args) : AstExpr(loc), Callee(Callee), Args(std::move(Args)) {}
    
    std::unique_ptr<InterpreterValue> eval(const Context& context) const {
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
            funcContext->setValue(protoArgs[i].Name, std::move(evaluatedArgs[i]));
        }

        return calleeFunc->eval(*funcContext);
    }

    std::unique_ptr<AstExpr> clone() const {
        std::vector<std::unique_ptr<AstExpr>> clonedArgs;
        for (const auto& arg : Args) {
            clonedArgs.push_back(arg->clone());
        }
        return std::make_unique<AstExprCall>(Location, Callee, std::move(clonedArgs));
    }
};

class AstExprLetIn : public AstExpr {
    std::string Variable;
    std::unique_ptr<AstExpr> Expr;
    std::unique_ptr<AstExpr> Body;
public:
    AstExprLetIn(const SourceLocation &loc,
                const std::string &Variable,
                std::unique_ptr<AstExpr> Expr,
                std::unique_ptr<AstExpr> Body) :
        AstExpr(loc), Variable(Variable), Expr(std::move(Expr)), Body(std::move(Body)) {}

    std::unique_ptr<InterpreterValue> eval(const Context& context) const {
        std::unique_ptr<InterpreterValue> evaluatedExpr = Expr->eval(context);
        if (!evaluatedExpr) {
            return nullptr;
        }

        std::unique_ptr<Context> newContext = context.clone();
        newContext->setValue(Variable, std::move(evaluatedExpr));

        return Body->eval(*newContext);
    }

    std::unique_ptr<AstExpr> clone() const {
        return std::make_unique<AstExprLetIn>(Location, Variable, Expr->clone(), Body->clone());
    }
};

enum class BinaryOpKindIntToInt {
    Add, Sub, Mul, Div,
};

template <BinaryOpKindIntToInt OpKind>
class AstExprBinaryIntToInt : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinaryIntToInt(const SourceLocation &loc,
                        std::unique_ptr<AstExpr> LHS,
                        std::unique_ptr<AstExpr> RHS) :
        AstExpr(loc), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    std::unique_ptr<InterpreterValue> eval(const Context& context) const {
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

    std::unique_ptr<AstExpr> clone() const {
        return std::make_unique<AstExprBinaryIntToInt<OpKind>>(Location, LHS->clone(), RHS->clone());
    }
};

template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>;
template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>;
template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>;
template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>;


enum class BinaryOpKindIntToBool {
    Eq, Neq, Leq, Lt, Geq, Gt
};

template <BinaryOpKindIntToBool OpKind>
class AstExprBinaryIntToBool : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinaryIntToBool(const SourceLocation &loc,
                            std::unique_ptr<AstExpr> LHS,
                            std::unique_ptr<AstExpr> RHS) :
        AstExpr(loc), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    std::unique_ptr<InterpreterValue> eval(const Context& context) const {
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

    std::unique_ptr<AstExpr> clone() const {
        return std::make_unique<AstExprBinaryIntToBool<OpKind>>(Location, LHS->clone(), RHS->clone());
    }
};

template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>;


enum class BinaryOpKindBoolToBool {
    And, Or
};

template <BinaryOpKindBoolToBool OpKind>
class AstExprBinaryBoolToBool : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinaryBoolToBool(const SourceLocation &loc,
                            std::unique_ptr<AstExpr> LHS,
                            std::unique_ptr<AstExpr> RHS) :
        AstExpr(loc), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    std::unique_ptr<InterpreterValue> eval(const Context& context) const {
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

    std::unique_ptr<AstExpr> clone() const {
        return std::make_unique<AstExprBinaryBoolToBool<OpKind>>(Location, LHS->clone(), RHS->clone());
    }

};

template class AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>;
template class AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>;


class AstExprMatchPath {
    SourceLocation Location;
public:
    std::unique_ptr<AstExpr> Guard;
    std::unique_ptr<AstExpr> Body;

    AstExprMatchPath(const SourceLocation &loc,
                        std::unique_ptr<AstExpr> guard,
                        std::unique_ptr<AstExpr> body) :
        Location(loc), Guard(std::move(guard)), Body(std::move(body)) {}
    const SourceLocation& getLocation() const { return Location; }

    std::unique_ptr<AstExprMatchPath> clone() const {
        return std::make_unique<AstExprMatchPath>(Location, Guard->clone(), Body->clone());
    }
};

class AstExprMatch : public AstExpr {
    std::vector<std::unique_ptr<AstExprMatchPath>> Paths;
public:
    AstExprMatch(const SourceLocation &loc, std::vector<std::unique_ptr<AstExprMatchPath>> Paths) :
        AstExpr(loc), Paths(std::move(Paths)) {}

    std::unique_ptr<InterpreterValue> eval(const Context& context) const {
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

    std::unique_ptr<AstExpr> clone() const {
        std::vector<std::unique_ptr<AstExprMatchPath>> clonedPaths;
        for (const auto& path : Paths) {
            clonedPaths.push_back(path->clone());
        }
        return std::make_unique<AstExprMatch>(Location, std::move(clonedPaths));
    }
};

#endif