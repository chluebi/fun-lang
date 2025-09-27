#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <utility>

#include "llvm/IR/Value.h"

#include "source_location.hpp"

// Forward declarations
class InterpreterValue;
class Context;

enum class BinaryOpKindIntToInt {
    Add, Sub, Mul, Div,
};

enum class BinaryOpKindIntToBool {
    Eq, Neq, Leq, Lt, Geq, Gt
};

enum class BinaryOpKindBoolToBool {
    And, Or
};

// --- Visitor Interface Definition ---
class AstExpr;
class AstExprConstLong;
class AstExprConstBool;
class AstExprVariable;
class AstExprCall;
class AstExprLetIn;
class AstExprMatch;

enum class BinaryOpKindIntToInt;
template <BinaryOpKindIntToInt OpKind> class AstExprBinaryIntToInt;

enum class BinaryOpKindIntToBool;
template <BinaryOpKindIntToBool OpKind> class AstExprBinaryIntToBool;

enum class BinaryOpKindBoolToBool;
template <BinaryOpKindBoolToBool OpKind> class AstExprBinaryBoolToBool;

class AstExprMatchPath;

class AstValueVisitor {
public:
    virtual ~AstValueVisitor() = default;

    virtual std::unique_ptr<InterpreterValue> visit(const AstExprConstLong& expr) const = 0;
    virtual std::unique_ptr<InterpreterValue> visit(const AstExprConstBool& expr) const = 0;
    virtual std::unique_ptr<InterpreterValue> visit(const AstExprVariable& expr) const = 0;
    virtual std::unique_ptr<InterpreterValue> visit(const AstExprCall& expr) const = 0;
    virtual std::unique_ptr<InterpreterValue> visit(const AstExprLetIn& expr) const = 0;
    virtual std::unique_ptr<InterpreterValue> visit(const AstExprMatch& expr) const = 0;

    virtual std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>& expr) const = 0;
    virtual std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>& expr) const = 0;
    virtual std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>& expr) const = 0;
    virtual std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>& expr) const = 0;

    virtual std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>& expr) const = 0;
    virtual std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>& expr) const = 0;
    virtual std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>& expr) const = 0;
    virtual std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>& expr) const = 0;
    virtual std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>& expr) const = 0;
    virtual std::unique_ptr<InterpreterValue> visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>& expr) const = 0;

    virtual std::unique_ptr<InterpreterValue> visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>& expr) const = 0;
    virtual std::unique_ptr<InterpreterValue> visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>& expr) const = 0;
};

class AstLLVMValueVisitor {
public:
    virtual ~AstLLVMValueVisitor() = default;

    virtual llvm::Value *visit(const AstExprConstLong& expr) const = 0;
    virtual llvm::Value *visit(const AstExprConstBool& expr) const = 0;
    virtual llvm::Value *visit(const AstExprVariable& expr) const = 0;
    virtual llvm::Value *visit(const AstExprCall& expr) const = 0;
    virtual llvm::Value *visit(const AstExprLetIn& expr) const = 0;
    virtual llvm::Value *visit(const AstExprMatch& expr) const = 0;

    virtual llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>& expr) const = 0;
    virtual llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>& expr) const = 0;
    virtual llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>& expr) const = 0;
    virtual llvm::Value *visit(const AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>& expr) const = 0;

    virtual llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>& expr) const = 0;
    virtual llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>& expr) const = 0;
    virtual llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>& expr) const = 0;
    virtual llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>& expr) const = 0;
    virtual llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>& expr) const = 0;
    virtual llvm::Value *visit(const AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>& expr) const = 0;

    virtual llvm::Value *visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>& expr) const = 0;
    virtual llvm::Value *visit(const AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>& expr) const = 0;
};


class AstExpr {
protected:
    SourceLocation Location;
public:
    AstExpr(const SourceLocation& loc);
    virtual ~AstExpr() = default;
    virtual std::unique_ptr<AstExpr> clone() const = 0;
    const SourceLocation& getLocation() const;

    virtual std::unique_ptr<InterpreterValue> accept(const AstValueVisitor& visitor) const = 0;
    virtual llvm::Value *accept(const AstLLVMValueVisitor& visitor) const = 0;
};

class AstExprConst : public AstExpr {
public:
    AstExprConst(const SourceLocation& loc);
};

class AstExprConstLong : public AstExprConst {
    long Value;
public:
    AstExprConstLong(const SourceLocation &loc, const long &Value);
    long getValue() const;
    std::unique_ptr<AstExpr> clone() const override;

    std::unique_ptr<InterpreterValue> accept(const AstValueVisitor& visitor) const override;
    llvm::Value *accept(const AstLLVMValueVisitor& visitor) const override;
};

class AstExprConstBool : public AstExprConst {
    bool Value;
public:
    AstExprConstBool(const SourceLocation &loc, const bool &Value);
    bool getValue() const;
    std::unique_ptr<AstExpr> clone() const override;

    std::unique_ptr<InterpreterValue> accept(const AstValueVisitor& visitor) const override;
    llvm::Value *accept(const AstLLVMValueVisitor& visitor) const override;
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
    const AstExpr* getBody() const;
    std::unique_ptr<AstFunction> clone() const;
};

class AstExprVariable : public AstExpr {
    std::string Name;
public:
    AstExprVariable(const SourceLocation &loc, const std::string &Name);
    const std::string& getName() const;
    std::unique_ptr<AstExpr> clone() const override;

    std::unique_ptr<InterpreterValue> accept(const AstValueVisitor& visitor) const override;
    llvm::Value *accept(const AstLLVMValueVisitor& visitor) const override;
};

class AstExprCall : public AstExpr {
    std::string Callee;
    std::vector<std::unique_ptr<AstExpr>> Args;
public:
    AstExprCall(const SourceLocation &loc, const std::string &Callee,
                std::vector<std::unique_ptr<AstExpr>> Args);
    const std::string& getCallee() const;
    const std::vector<std::unique_ptr<AstExpr>>& getArgs() const;
    std::unique_ptr<AstExpr> clone() const override;

    std::unique_ptr<InterpreterValue> accept(const AstValueVisitor& visitor) const override;
    llvm::Value *accept(const AstLLVMValueVisitor& visitor) const override;
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
    const std::string& getVariable() const;
    const AstExpr* getExpr() const;
    const AstExpr* getBody() const;
    std::unique_ptr<AstExpr> clone() const override;

    std::unique_ptr<InterpreterValue> accept(const AstValueVisitor& visitor) const override;
    llvm::Value *accept(const AstLLVMValueVisitor& visitor) const override;
};



template <BinaryOpKindIntToInt OpKind>
class AstExprBinaryIntToInt : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinaryIntToInt(const SourceLocation &loc,
                        std::unique_ptr<AstExpr> LHS,
                        std::unique_ptr<AstExpr> RHS);
    const AstExpr* getLHS() const;
    const AstExpr* getRHS() const;
    std::unique_ptr<AstExpr> clone() const override;

    std::unique_ptr<InterpreterValue> accept(const AstValueVisitor& visitor) const override;
    llvm::Value *accept(const AstLLVMValueVisitor& visitor) const override;
};

extern template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>;
extern template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>;
extern template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>;
extern template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>;



template <BinaryOpKindIntToBool OpKind>
class AstExprBinaryIntToBool : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinaryIntToBool(const SourceLocation &loc,
                            std::unique_ptr<AstExpr> LHS,
                            std::unique_ptr<AstExpr> RHS);
    const AstExpr* getLHS() const;
    const AstExpr* getRHS() const;
    std::unique_ptr<AstExpr> clone() const override;

    std::unique_ptr<InterpreterValue> accept(const AstValueVisitor& visitor) const override;
    llvm::Value *accept(const AstLLVMValueVisitor& visitor) const override;
};

extern template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>;
extern template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>;
extern template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>;
extern template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>;
extern template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>;
extern template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>;



template <BinaryOpKindBoolToBool OpKind>
class AstExprBinaryBoolToBool : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinaryBoolToBool(const SourceLocation &loc,
                            std::unique_ptr<AstExpr> LHS,
                            std::unique_ptr<AstExpr> RHS);
    const AstExpr* getLHS() const;
    const AstExpr* getRHS() const;
    std::unique_ptr<AstExpr> clone() const override;

    std::unique_ptr<InterpreterValue> accept(const AstValueVisitor& visitor) const override;
    llvm::Value *accept(const AstLLVMValueVisitor& visitor) const override;
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
    const AstExpr* getGuard() const;
    const AstExpr* getBody() const;
    std::unique_ptr<AstExprMatchPath> clone() const;
};

class AstExprMatch : public AstExpr {
    std::vector<std::unique_ptr<AstExprMatchPath>> Paths;
public:
    AstExprMatch(const SourceLocation &loc, std::vector<std::unique_ptr<AstExprMatchPath>> Paths);
    const std::vector<std::unique_ptr<AstExprMatchPath>>& getPaths() const;
    std::unique_ptr<AstExpr> clone() const override;

    std::unique_ptr<InterpreterValue> accept(const AstValueVisitor& visitor) const override;
    llvm::Value *accept(const AstLLVMValueVisitor& visitor) const override;
};

#endif