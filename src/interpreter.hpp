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

class Interpreter {
public:
    std::unique_ptr<InterpreterValue> eval(const AstExpr& expr, const Context& context) const;
private:
    std::unique_ptr<InterpreterValue> evalBinaryIntToInt(
        const AstExpr& lhs, const AstExpr& rhs, const Context& context,
        BinaryOpKindIntToInt op) const;
    std::unique_ptr<InterpreterValue> evalBinaryIntToBool(
        const AstExpr& lhs, const AstExpr& rhs, const Context& context,
        BinaryOpKindIntToBool op) const;
    std::unique_ptr<InterpreterValue> evalBinaryBoolToBool(
        const AstExpr& lhs, const AstExpr& rhs, const Context& context,
        BinaryOpKindBoolToBool op) const;
};

#endif