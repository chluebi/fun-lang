#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>

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
    std::unique_ptr<InterpreterValue> clone() const override;
};

class InterpreterValueBool : public InterpreterValue {
    bool Value;
public:
    InterpreterValueBool(const bool &Value) : Value(Value) {}
    bool getValue() const { return Value; }
    std::unique_ptr<InterpreterValue> clone() const override;
};

// Base class for all AST nodes
class AstExpr {
public:
    virtual ~AstExpr() = default;
    virtual std::unique_ptr<InterpreterValue> eval(const Context& context) const = 0;
    virtual std::unique_ptr<AstExpr> clone() const = 0;
};

// Constant expressions
class AstExprConst : public AstExpr {
protected:
    virtual std::unique_ptr<InterpreterValue> getValue() const = 0;
public:
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
};

class AstExprConstLong : public AstExprConst {
    long Value;
public:
    AstExprConstLong(const long &Value) : Value(Value) {}
    std::unique_ptr<InterpreterValue> getValue() const override;
    std::unique_ptr<AstExpr> clone() const override;
};

class AstExprConstBool : public AstExprConst {
    bool Value;
public:
    AstExprConstBool(const bool &Value) : Value(Value) {}
    std::unique_ptr<InterpreterValue> getValue() const override;
    std::unique_ptr<AstExpr> clone() const override;
};

// Function prototype (name and arguments)
class AstPrototype {
    std::string Name;
    std::vector<std::string> Args;
public:
    AstPrototype(const std::string &Name, std::vector<std::string> Args)
        : Name(Name), Args(std::move(Args)) {}
    const std::string &getName() const { return Name; }
    const std::vector<std::string> &getArgs() const { return Args; }
};

// Function definition
class AstFunction {
    std::unique_ptr<AstPrototype> Proto;
    std::unique_ptr<AstExpr> Body;
public:
    AstFunction(std::unique_ptr<AstPrototype> Proto, std::unique_ptr<AstExpr> Body);
    const AstPrototype* getPrototype() const { return Proto.get(); }
    std::unique_ptr<InterpreterValue> eval(const Context& context) const;
    std::unique_ptr<AstFunction> clone() const;
};

// Execution context for variables and functions
class Context {
private:
    std::unordered_map<std::string, std::unique_ptr<InterpreterValue>> variables;
    std::unordered_map<std::string, std::unique_ptr<AstFunction>> functions;
public:
    InterpreterValue* getValue(const std::string& name) const;
    void setValue(const std::string& name, std::unique_ptr<InterpreterValue> value);
    const AstFunction* getFunction(const std::string& name) const;
    void addFunction(std::unique_ptr<AstFunction> func);
    std::unique_ptr<Context> cloneFunctionContext() const;
    std::unique_ptr<Context> clone() const;
};

// AST nodes for various expressions
class AstExprVariable : public AstExpr {
    std::string Name;
public:
    AstExprVariable(const std::string &Name) : Name(Name) {}
    const std::string& getName() const { return Name; }
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
    std::unique_ptr<AstExpr> clone() const override;
};

class AstExprCall : public AstExpr {
    std::string Callee;
    std::vector<std::unique_ptr<AstExpr>> Args;
public:
    AstExprCall(const std::string &Callee,
                std::vector<std::unique_ptr<AstExpr>> Args);
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
    std::unique_ptr<AstExpr> clone() const override;
};

class AstExprLetIn : public AstExpr {
    std::string Variable;
    std::unique_ptr<AstExpr> Expr;
    std::unique_ptr<AstExpr> Body;
public:
    AstExprLetIn(const std::string &Variable,
                std::unique_ptr<AstExpr> Expr,
                std::unique_ptr<AstExpr> Body);
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
    std::unique_ptr<AstExpr> clone() const override;
};

enum class BinaryOpKindIntToInt {
    Add, Sub, Mul, Div,
};

template <BinaryOpKindIntToInt OpKind>
class AstExprBinaryIntToInt : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinaryIntToInt(std::unique_ptr<AstExpr> LHS, std::unique_ptr<AstExpr> RHS);
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
    std::unique_ptr<AstExpr> clone() const override;
};

enum class BinaryOpKindIntToBool {
    Eq, Neq, Leq, Lt, Geq, Gt
};

template <BinaryOpKindIntToBool OpKind>
class AstExprBinaryIntToBool : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinaryIntToBool(std::unique_ptr<AstExpr> LHS, std::unique_ptr<AstExpr> RHS);
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
    std::unique_ptr<AstExpr> clone() const override;
};

enum class BinaryOpKindBoolToBool {
    And, Or
};

template <BinaryOpKindBoolToBool OpKind>
class AstExprBinaryBoolToBool : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinaryBoolToBool(std::unique_ptr<AstExpr> LHS, std::unique_ptr<AstExpr> RHS);
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
    std::unique_ptr<AstExpr> clone() const override;
};

class AstExprMatchPath {
public:
    std::unique_ptr<AstExpr> Guard;
    std::unique_ptr<AstExpr> Body;

    AstExprMatchPath(std::unique_ptr<AstExpr> guard, std::unique_ptr<AstExpr> body);
    std::unique_ptr<AstExprMatchPath> clone() const;
};

class AstExprMatch : public AstExpr {
    std::vector<std::unique_ptr<AstExprMatchPath>> Paths;
public:
    AstExprMatch(std::vector<std::unique_ptr<AstExprMatchPath>> Paths);
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
    std::unique_ptr<AstExpr> clone() const override;
};

#endif