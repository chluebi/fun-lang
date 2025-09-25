#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>
#include <utility>

#include "source_location.hpp"

// Forward declarations
class Context;
class AstExpr;
class AstFunction;
class AstPrototype;


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



class AstExpr {
protected:
    SourceLocation Location;
public:
    AstExpr(const SourceLocation& loc);
    virtual ~AstExpr() = default;
    virtual std::unique_ptr<InterpreterValue> eval(const Context& context) const = 0;
    virtual std::unique_ptr<AstExpr> clone() const = 0;
    const SourceLocation& getLocation() const;
};

class AstExprConst : public AstExpr {
protected:
    virtual std::unique_ptr<InterpreterValue> getValue() const = 0;
public:
    AstExprConst(const SourceLocation& loc);
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
};

class AstExprConstLong : public AstExprConst {
    long Value;
protected:
    std::unique_ptr<InterpreterValue> getValue() const override;
public:
    AstExprConstLong(const SourceLocation &loc, const long &Value);
    std::unique_ptr<AstExpr> clone() const override;
};

class AstExprConstBool : public AstExprConst {
    bool Value;
protected:
    std::unique_ptr<InterpreterValue> getValue() const override;
public:
    AstExprConstBool(const SourceLocation &loc, const bool &Value);
    std::unique_ptr<AstExpr> clone() const override;
};



struct AstArg {
    SourceLocation Location;
    std::string Name;
    AstArg(const SourceLocation& loc, const std::string& name);
};

class AstPrototype {
    SourceLocation Location;
    std::string Name;
    std::vector<AstArg> Args;
public:
    AstPrototype(const SourceLocation& loc, const std::string& name, std::vector<AstArg> args);
    const SourceLocation& getLocation() const;
    const std::string& getName() const;
    const std::vector<AstArg>& getArgs() const;
};

class AstFunction {
    SourceLocation Location;
    std::unique_ptr<AstPrototype> Proto;
    std::unique_ptr<AstExpr> Body;
public:
    AstFunction(const SourceLocation &loc, std::unique_ptr<AstPrototype> Proto, std::unique_ptr<AstExpr> Body);
    const SourceLocation& getLocation() const;
    const AstPrototype* getPrototype() const;
    std::unique_ptr<InterpreterValue> eval(const Context& context) const;
    std::unique_ptr<AstFunction> clone() const;
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



class AstExprVariable : public AstExpr {
    std::string Name;
public:
    AstExprVariable(const SourceLocation &loc, const std::string &Name);
    const std::string& getName() const;
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
    std::unique_ptr<AstExpr> clone() const override;
};

class AstExprCall : public AstExpr {
    std::string Callee;
    std::vector<std::unique_ptr<AstExpr>> Args;
public:
    AstExprCall(const SourceLocation &loc, const std::string &Callee,
                std::vector<std::unique_ptr<AstExpr>> Args);
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
    std::unique_ptr<AstExpr> clone() const override;
};

class AstExprLetIn : public AstExpr {
    std::string Variable;
    std::unique_ptr<AstExpr> Expr;
    std::unique_ptr<AstExpr> Body;
public:
    AstExprLetIn(const SourceLocation &loc,
                const std::string &Variable,
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
    AstExprBinaryIntToInt(const SourceLocation &loc,
                        std::unique_ptr<AstExpr> LHS,
                        std::unique_ptr<AstExpr> RHS);
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
    std::unique_ptr<AstExpr> clone() const override;
};


extern template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>;
extern template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>;
extern template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>;
extern template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>;


enum class BinaryOpKindIntToBool {
    Eq, Neq, Leq, Lt, Geq, Gt
};

template <BinaryOpKindIntToBool OpKind>
class AstExprBinaryIntToBool : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinaryIntToBool(const SourceLocation &loc,
                            std::unique_ptr<AstExpr> LHS,
                            std::unique_ptr<AstExpr> RHS);
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
    std::unique_ptr<AstExpr> clone() const override;
};


extern template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>;
extern template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>;
extern template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>;
extern template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>;
extern template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>;
extern template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>;


enum class BinaryOpKindBoolToBool {
    And, Or
};

template <BinaryOpKindBoolToBool OpKind>
class AstExprBinaryBoolToBool : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinaryBoolToBool(const SourceLocation &loc,
                            std::unique_ptr<AstExpr> LHS,
                            std::unique_ptr<AstExpr> RHS);
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
    std::unique_ptr<AstExpr> clone() const override;
};


extern template class AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>;
extern template class AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>;

class AstExprMatchPath {
    SourceLocation Location;
public:
    std::unique_ptr<AstExpr> Guard;
    std::unique_ptr<AstExpr> Body;

    AstExprMatchPath(const SourceLocation &loc,
                        std::unique_ptr<AstExpr> guard,
                        std::unique_ptr<AstExpr> body);
    const SourceLocation& getLocation() const;
    std::unique_ptr<AstExprMatchPath> clone() const;
};

class AstExprMatch : public AstExpr {
    std::vector<std::unique_ptr<AstExprMatchPath>> Paths;
public:
    AstExprMatch(const SourceLocation &loc, std::vector<std::unique_ptr<AstExprMatchPath>> Paths);
    std::unique_ptr<InterpreterValue> eval(const Context& context) const override;
    std::unique_ptr<AstExpr> clone() const override;
};

#endif