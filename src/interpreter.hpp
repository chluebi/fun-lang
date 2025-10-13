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

class InterpreterValueArray : public InterpreterValue {
    std::unique_ptr<Type> ElementType;
    std::vector<std::unique_ptr<InterpreterValue>> Value;
public:
    InterpreterValueArray(const std::vector<std::unique_ptr<InterpreterValue>> &Value);
    const std::vector<std::unique_ptr<InterpreterValue>>& getValue() const;
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

    void debugPrint() const {
        std::cout << "--- Context Debug Print ---" << std::endl;

        // Print Variables
        std::cout << "Variables (" << variables.size() << "):" << std::endl;
        for (const auto& pair : variables) {
            // Assume InterpreterValue has a 'toString()' or similar debug method
            // or you implement a way to print its value.
            std::cout << "  - Name: '" << pair.first << "', Value: [Value details here]" << std::endl;
        }
        
        // Print Functions
        std::cout << "Functions (" << functions.size() << "):" << std::endl;
        for (const auto& pair : functions) {
            // Assume AstFunction has a 'getName()' or similar debug method
            std::cout << "  - Name: '" << pair.first << "', Function: [Function details here]" << std::endl;
        }

        std::cout << "--- End Context Debug Print ---" << std::endl;
    }
};


class Interpreter : public AstValueVisitor {
private:
    // Mark as mutable to allow modification in const methods
    mutable const Context* CurrentContext = nullptr;
    
    // Allow ContextGuard to access private member CurrentContext
    friend class ContextGuard;

public:
    Interpreter(const Context& initialContext);
    std::unique_ptr<InterpreterValue> eval(const AstExpr& expr) const;
private:
    std::unique_ptr<InterpreterValue> visit(const AstExprConstLong& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprConstBool& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprConstArray& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprVariable& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprCall& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprLetIn& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprMatch& expr) const override;

    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>& expr) const override;

    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>& expr) const override;

    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>& expr) const override;
    std::unique_ptr<InterpreterValue> visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>& expr) const override;

    std::unique_ptr<InterpreterValue> evalBinaryIntToInt(
        const AstExpr& lhs, const AstExpr& rhs, BinaryOpKindIntToInt op) const;
    std::unique_ptr<InterpreterValue> evalBinaryIntToBool(
        const AstExpr& lhs, const AstExpr& rhs, BinaryOpKindIntToBool op) const;
    std::unique_ptr<InterpreterValue> evalBinaryBoolToBool(
        const AstExpr& lhs, const AstExpr& rhs, BinaryOpKindBoolToBool op) const;

    std::unique_ptr<InterpreterValue> runWithContext(const AstExpr& expr, const Context& newContext) const;
};

// RAII helper to save and restore the interpreter's context pointer.
class ContextGuard {
private:
    Interpreter* CurrentInterpreter; 
    const Context* SavedContext;
public:
    ContextGuard(Interpreter* interpreter, const Context* newContext)
        : CurrentInterpreter(interpreter), SavedContext(interpreter->CurrentContext) 
    {
        CurrentInterpreter->CurrentContext = newContext; 
    }

    ~ContextGuard() {
        CurrentInterpreter->CurrentContext = SavedContext; 
    }
};

#endif
