#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>

// Forward declarations
class AstExpr;

// Context
class Context {
public:
    std::optional<long> getValue(const std::string& name) const {
        auto it = variables.find(name);
        if (it != variables.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    void setValue(const std::string& name, long value) {
        variables[name] = value;
    }
private:
    std::unordered_map<std::string, long> variables;
};

// The base expression with virtual methods
class AstExpr {
public:
    virtual ~AstExpr() = default;
    virtual std::unique_ptr<AstExpr> eval(const Context& context) const = 0;
};

// Expression Types
class AstExprConst : public AstExpr {
    long Value;
public:
    AstExprConst(const long &Value) : Value(Value) {}
    long getValue() const { return Value; }
    std::unique_ptr<AstExpr> eval(const Context& context) const override {
        return std::make_unique<AstExprConst>(Value);
    }
};

class AstExprVariable : public AstExpr {
    std::string Name;
public:
    AstExprVariable(const std::string &Name) : Name(Name) {}
    const std::string& getName() const { return Name; }
    std::unique_ptr<AstExpr> eval(const Context& context) const override {
        auto value = context.getValue(Name);
        if (value.has_value()) {
            return std::make_unique<AstExprConst>(value.value());
        }
        return std::make_unique<AstExprVariable>(Name);
    }
};

enum class BinaryOpKind {
    Add,
    Sub,
    Mul,
    Div,
};

class AstExprBinaryBase : public AstExpr {
public:
    virtual BinaryOpKind getOpKind() const = 0;
};

template <BinaryOpKind OpKind>
class AstExprBinary : public AstExprBinaryBase {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinary(std::unique_ptr<AstExpr> LHS, std::unique_ptr<AstExpr> RHS)
        : LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    BinaryOpKind getOpKind() const override { return OpKind; }

    std::unique_ptr<AstExpr> eval(const Context& context) const override {
        auto evaluatedLHS = LHS->eval(context);
        auto evaluatedRHS = RHS->eval(context);

        AstExprConst* lhsConst = dynamic_cast<AstExprConst*>(evaluatedLHS.get());
        AstExprConst* rhsConst = dynamic_cast<AstExprConst*>(evaluatedRHS.get());

        if (lhsConst && rhsConst) {
            long result;
            long lhsVal = lhsConst->getValue();
            long rhsVal = rhsConst->getValue();

            if constexpr (OpKind == BinaryOpKind::Add) {
                result = lhsVal + rhsVal;
            } else if constexpr (OpKind == BinaryOpKind::Sub) {
                result = lhsVal - rhsVal;
            } else if constexpr (OpKind == BinaryOpKind::Mul) {
                result = lhsVal * rhsVal;
            } else if constexpr (OpKind == BinaryOpKind::Div) {
                if (rhsVal != 0) {
                    result = lhsVal / rhsVal;
                } else {
                    // Division by zero, return the original expression
                    return std::make_unique<AstExprBinary<OpKind>>(
                        std::move(evaluatedLHS),
                        std::move(evaluatedRHS)
                    );
                }
            }
            return std::make_unique<AstExprConst>(result);
        }

        return std::make_unique<AstExprBinary<OpKind>>(
            std::move(evaluatedLHS),
            std::move(evaluatedRHS)
        );
    }
};

class AstExprCall : public AstExpr {
    std::string Callee;
    std::vector<std::unique_ptr<AstExpr>> Args;
public:
    AstExprCall(const std::string &Callee,
                std::vector<std::unique_ptr<AstExpr>> Args)
        : Callee(Callee), Args(std::move(Args)) {}
    
    std::unique_ptr<AstExpr> eval(const Context& context) const override {
        std::vector<std::unique_ptr<AstExpr>> evaluatedArgs;
        for (const auto& arg : Args) {
            evaluatedArgs.push_back(arg->eval(context));
        }
        return std::make_unique<AstExprCall>(Callee, std::move(evaluatedArgs));
    }
};

class AstPrototype {
    std::string Name;
    std::vector<std::string> Args;
public:
    AstPrototype(const std::string &Name, std::vector<std::string> Args)
        : Name(Name), Args(std::move(Args)) {}
    const std::string &getName() const { return Name; }
};

class AstFunction {
    std::unique_ptr<AstPrototype> Proto;
    std::unique_ptr<AstExpr> Body;
public:
    AstFunction(std::unique_ptr<AstPrototype> Proto,
                std::unique_ptr<AstExpr> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}

    std::unique_ptr<AstExpr> eval(const Context& context) const {
        return Body->eval(context);
    }
};

int main(int argc, char *argv[]) {
    auto expr = std::make_unique<AstExprBinary<BinaryOpKind::Mul>>(
        std::make_unique<AstExprBinary<BinaryOpKind::Add>>(
            std::make_unique<AstExprConst>(5),
            std::make_unique<AstExprVariable>("y")
        ),
        std::make_unique<AstExprConst>(2)
    );

    AstFunction func(
        std::make_unique<AstPrototype>("MyFunction", std::vector<std::string>{"y"}),
        std::move(expr)
    );
    
    Context context;
    context.setValue("y", 3);
    
    auto evaluatedExpr = func.eval(context);
    
    AstExprConst* resultConst = dynamic_cast<AstExprConst*>(evaluatedExpr.get());
    if (resultConst) {
        std::cout << "Result: " << resultConst->getValue() << std::endl; // Should print 16
    } else {
        std::cout << "Could not fully evaluate the expression." << std::endl;
    }

    return 0;
}