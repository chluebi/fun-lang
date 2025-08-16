#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>
#include <utility>

// Forward declarations
class Context;
class AstExpr;
class AstFunction;
class AstPrototype;

class AstPrototype {
    std::string Name;
    std::vector<std::string> Args;
public:
    AstPrototype(const std::string &Name, std::vector<std::string> Args)
        : Name(Name), Args(std::move(Args)) {}
    const std::string &getName() const { return Name; }
    const std::vector<std::string> &getArgs() const { return Args; }
};

class AstExpr {
public:
    virtual ~AstExpr() = default;
    virtual std::unique_ptr<AstExpr> eval(const Context& context) const = 0;
    virtual std::unique_ptr<AstExpr> clone() const = 0;
};

class AstExprConst : public AstExpr {
    long Value;
public:
    AstExprConst(const long &Value) : Value(Value) {}
    long getValue() const { return Value; }
    std::unique_ptr<AstExpr> eval(const Context& context) const override {
        return std::make_unique<AstExprConst>(Value);
    }
    std::unique_ptr<AstExpr> clone() const override {
        return std::make_unique<AstExprConst>(Value);
    }
};

enum class BinaryOpKind {
    Add, Sub, Mul, Div,
};

class AstFunction {
    std::unique_ptr<AstPrototype> Proto;
    std::unique_ptr<AstExpr> Body;
public:
    AstFunction(std::unique_ptr<AstPrototype> Proto, std::unique_ptr<AstExpr> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}

    const AstPrototype* getPrototype() const { return Proto.get(); }
    std::unique_ptr<AstExpr> eval(const Context& context) const;
};

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
private:
    std::unordered_map<std::string, long> variables;
    std::unordered_map<std::string, std::unique_ptr<AstFunction>> functions;
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
    
    std::unique_ptr<AstExpr> eval(const Context& context) const override {
        const AstFunction* calleeFunc = context.getFunction(Callee);
        
        std::vector<std::unique_ptr<AstExpr>> evaluatedArgs;
        bool allArgsEvaluated = true;
        for (const auto& arg : this->Args) {
            auto evaluated = arg->eval(context);
            if (dynamic_cast<AstExprConst*>(evaluated.get())) {
                evaluatedArgs.push_back(std::move(evaluated));
            } else {
                allArgsEvaluated = false;
                evaluatedArgs.push_back(std::move(evaluated));
            }
        }
        
        if (!calleeFunc || !allArgsEvaluated) {
            return std::make_unique<AstExprCall>(Callee, std::move(evaluatedArgs));
        }
        
        std::vector<long> evaluatedArgValues;
        for(const auto& arg : evaluatedArgs) {
             evaluatedArgValues.push_back(dynamic_cast<AstExprConst*>(arg.get())->getValue());
        }

        Context funcContext;
        const auto& protoArgs = calleeFunc->getPrototype()->getArgs();
        if (protoArgs.size() != evaluatedArgValues.size()) {
            return nullptr;
        }
        for (size_t i = 0; i < protoArgs.size(); ++i) {
            funcContext.setValue(protoArgs[i], evaluatedArgValues[i]);
        }
        return calleeFunc->eval(funcContext);
    }
    
    std::unique_ptr<AstExpr> clone() const override {
        std::vector<std::unique_ptr<AstExpr>> clonedArgs;
        for (const auto& arg : Args) {
            clonedArgs.push_back(arg->clone());
        }
        return std::make_unique<AstExprCall>(Callee, std::move(clonedArgs));
    }
};

template <BinaryOpKind OpKind>
class AstExprBinary : public AstExpr {
    std::unique_ptr<AstExpr> LHS, RHS;
public:
    AstExprBinary(std::unique_ptr<AstExpr> LHS, std::unique_ptr<AstExpr> RHS)
        : LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    
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

    std::unique_ptr<AstExpr> clone() const override {
        return std::make_unique<AstExprBinary<OpKind>>(
            LHS->clone(),
            RHS->clone()
        );
    }
};

std::unique_ptr<AstExpr> AstFunction::eval(const Context& context) const {
    return Body->eval(context);
}

int main() {
    auto addFuncBody = std::make_unique<AstExprBinary<BinaryOpKind::Add>>(
        std::make_unique<AstExprVariable>("x"),
        std::make_unique<AstExprVariable>("y")
    );
    auto addFuncProto = std::make_unique<AstPrototype>("add", std::vector<std::string>{"x", "y"});
    auto addFunc = std::make_unique<AstFunction>(std::move(addFuncProto), std::move(addFuncBody));

    std::vector<std::unique_ptr<AstExpr>> args = std::vector<std::unique_ptr<AstExpr>>{};
    args.push_back(std::make_unique<AstExprConst>(5));
    args.push_back(std::make_unique<AstExprConst>(3));

    auto callExpr = std::make_unique<AstExprCall>("add", std::move(args));

    auto mainFuncBody = std::make_unique<AstExprBinary<BinaryOpKind::Sub>>(
        callExpr->clone(),
        callExpr->clone()
    );
    auto mainFuncProto = std::make_unique<AstPrototype>("main", std::vector<std::string>{});
    AstFunction mainFunc(std::move(mainFuncProto), std::move(mainFuncBody));

    Context globalContext;
    globalContext.addFunction(std::move(addFunc));

    auto evaluatedExpr = mainFunc.eval(globalContext);
    
    AstExprConst* resultConst = dynamic_cast<AstExprConst*>(evaluatedExpr.get());
    if (resultConst) {
        std::cout << "Result: " << resultConst->getValue() << std::endl;
    } else {
        std::cout << "Could not fully evaluate the expression." << std::endl;
    }

    return 0;
}