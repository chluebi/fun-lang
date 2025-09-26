#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>
#include "ast.hpp"

class InterpreterValue {
public:
    virtual ~InterpreterValue() = default;
    virtual std::unique_ptr<InterpreterValue> clone() const = 0;
};

class InterpreterValueLong : public InterpreterValue {
    long Value;
public:
    InterpreterValueLong(const long &Value);
    long getValue() const;
    std::unique_ptr<InterpreterValue> clone() const override;
};

class InterpreterValueBool : public InterpreterValue {
    bool Value;
public:
    InterpreterValueBool(const bool &Value);
    bool getValue() const;
    std::unique_ptr<InterpreterValue> clone() const override;
};

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

class Interpreter : public AstVisitor { // Inherit from AstVisitor
private:
    const Context* CurrentContext = nullptr;
public:
    std::unique_ptr<InterpreterValue> eval(const AstExpr& expr, const Context& context) const;
private:
    std::unique_ptr<InterpreterValue> visit(const AstExprConstLong& expr) override;
    std::unique_ptr<InterpreterValue> visit(const AstExprConstBool& expr) override;
    std::unique_ptr<InterpreterValue> visit(const AstExprVariable& expr) override;
    std::unique_ptr<InterpreterValue> visit(const AstExprCall& expr) override;
    std::unique_ptr<InterpreterValue> visit(const AstExprLetIn& expr) override;
    std::unique_ptr<InterpreterValue> visit(const AstExprMatch& expr) override;

    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>& expr) override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>& expr) override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>& expr) override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>& expr) override;

    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>& expr) override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>& expr) override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>& expr) override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>& expr) override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>& expr) override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>& expr) override;

    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>& expr) override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>& expr) override;

    std::unique_ptr<InterpreterValue> evalBinaryIntToInt(
        const AstExpr& lhs, const AstExpr& rhs, BinaryOpKindIntToInt op) const;
    std::unique_ptr<InterpreterValue> evalBinaryIntToBool(
        const AstExpr& lhs, const AstExpr& rhs, BinaryOpKindIntToBool op) const;
    std::unique_ptr<InterpreterValue> evalBinaryBoolToBool(
        const AstExpr& lhs, const AstExpr& rhs, BinaryOpKindBoolToBool op) const;

    std::unique_ptr<InterpreterValue> runWithContext(const AstExpr& expr, const Context& newContext) const;
};

#endif