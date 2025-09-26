#include "ast.hpp"
#include "interpreter.hpp"

AstExpr::AstExpr(const SourceLocation& loc) : Location(loc) {}
const SourceLocation& AstExpr::getLocation() const { return Location; }

AstExprConst::AstExprConst(const SourceLocation& loc) : AstExpr(loc) {}

AstExprConstLong::AstExprConstLong(const SourceLocation &loc, const long &Value) : AstExprConst(loc), Value(Value) {}
long AstExprConstLong::getValue() const { return Value; }
std::unique_ptr<AstExpr> AstExprConstLong::clone() const {
    return std::make_unique<AstExprConstLong>(Location, Value);
}
std::unique_ptr<InterpreterValue> AstExprConstLong::accept(AstVisitor& visitor) const {
    return visitor.visit(*this);
}

AstExprConstBool::AstExprConstBool(const SourceLocation &loc, const bool &Value) : AstExprConst(loc), Value(Value) {}
bool AstExprConstBool::getValue() const { return Value; }
std::unique_ptr<AstExpr> AstExprConstBool::clone() const {
    return std::make_unique<AstExprConstBool>(Location, Value);
}
std::unique_ptr<InterpreterValue> AstExprConstBool::accept(AstVisitor& visitor) const {
    return visitor.visit(*this);
}

AstArg::AstArg(const SourceLocation& loc, const std::string& name)
    : Location(loc), Name(name) {}

AstPrototype::AstPrototype(const SourceLocation& loc, const std::string& name, std::vector<AstArg> args)
    : Location(loc), Name(name), Args(std::move(args)) {}
const SourceLocation& AstPrototype::getLocation() const { return Location; }
const std::string& AstPrototype::getName() const { return Name; }
const std::vector<AstArg>& AstPrototype::getArgs() const { return Args; }

AstFunction::AstFunction(const SourceLocation &loc, std::unique_ptr<AstPrototype> Proto, std::unique_ptr<AstExpr> Body)
    : Location(loc), Proto(std::move(Proto)), Body(std::move(Body)) {}
const SourceLocation& AstFunction::getLocation() const { return Location; }
const AstPrototype* AstFunction::getPrototype() const { return Proto.get(); }
const AstExpr* AstFunction::getBody() const { return Body.get(); }

std::unique_ptr<AstFunction> AstFunction::clone() const {
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

AstExprVariable::AstExprVariable(const SourceLocation &loc, const std::string &Name) : AstExpr(loc), Name(Name) {}
const std::string& AstExprVariable::getName() const { return Name; }
std::unique_ptr<AstExpr> AstExprVariable::clone() const {
    return std::make_unique<AstExprVariable>(Location, Name);
}
std::unique_ptr<InterpreterValue> AstExprVariable::accept(AstVisitor& visitor) const {
    return visitor.visit(*this);
}

AstExprCall::AstExprCall(const SourceLocation &loc, const std::string &Callee,
            std::vector<std::unique_ptr<AstExpr>> Args) : AstExpr(loc), Callee(Callee), Args(std::move(Args)) {}
const std::string& AstExprCall::getCallee() const { return Callee; }
const std::vector<std::unique_ptr<AstExpr>>& AstExprCall::getArgs() const { return Args; }
std::unique_ptr<AstExpr> AstExprCall::clone() const {
    std::vector<std::unique_ptr<AstExpr>> clonedArgs;
    for (const auto& arg : Args) {
        clonedArgs.push_back(arg->clone());
    }
    return std::make_unique<AstExprCall>(Location, Callee, std::move(clonedArgs));
}
std::unique_ptr<InterpreterValue> AstExprCall::accept(AstVisitor& visitor) const {
    return visitor.visit(*this);
}

AstExprLetIn::AstExprLetIn(const SourceLocation &loc,
            const std::string &Variable,
            std::unique_ptr<AstExpr> Expr,
            std::unique_ptr<AstExpr> Body) :
    AstExpr(loc), Variable(Variable), Expr(std::move(Expr)), Body(std::move(Body)) {}
const std::string& AstExprLetIn::getVariable() const { return Variable; }
const AstExpr* AstExprLetIn::getExpr() const { return Expr.get(); }
const AstExpr* AstExprLetIn::getBody() const { return Body.get(); }
std::unique_ptr<AstExpr> AstExprLetIn::clone() const {
    return std::make_unique<AstExprLetIn>(Location, Variable, Expr->clone(), Body->clone());
}
std::unique_ptr<InterpreterValue> AstExprLetIn::accept(AstVisitor& visitor) const {
    return visitor.visit(*this);
}

template <BinaryOpKindIntToInt OpKind>
AstExprBinaryIntToInt<OpKind>::AstExprBinaryIntToInt(const SourceLocation &loc,
                        std::unique_ptr<AstExpr> LHS,
                        std::unique_ptr<AstExpr> RHS) :
    AstExpr(loc), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
template <BinaryOpKindIntToInt OpKind>
const AstExpr* AstExprBinaryIntToInt<OpKind>::getLHS() const { return LHS.get(); }
template <BinaryOpKindIntToInt OpKind>
const AstExpr* AstExprBinaryIntToInt<OpKind>::getRHS() const { return RHS.get(); }
template <BinaryOpKindIntToInt OpKind>
std::unique_ptr<AstExpr> AstExprBinaryIntToInt<OpKind>::clone() const {
    return std::make_unique<AstExprBinaryIntToInt<OpKind>>(Location, LHS->clone(), RHS->clone());
}
template <BinaryOpKindIntToInt OpKind>
std::unique_ptr<InterpreterValue> AstExprBinaryIntToInt<OpKind>::accept(AstVisitor& visitor) const {
    return visitor.visit(*this);
}
template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>;
template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>;
template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>;
template class AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>;

template <BinaryOpKindIntToBool OpKind>
AstExprBinaryIntToBool<OpKind>::AstExprBinaryIntToBool(const SourceLocation &loc,
                        std::unique_ptr<AstExpr> LHS,
                        std::unique_ptr<AstExpr> RHS) :
    AstExpr(loc), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
template <BinaryOpKindIntToBool OpKind>
const AstExpr* AstExprBinaryIntToBool<OpKind>::getLHS() const { return LHS.get(); }
template <BinaryOpKindIntToBool OpKind>
const AstExpr* AstExprBinaryIntToBool<OpKind>::getRHS() const { return RHS.get(); }
template <BinaryOpKindIntToBool OpKind>
std::unique_ptr<AstExpr> AstExprBinaryIntToBool<OpKind>::clone() const {
    return std::make_unique<AstExprBinaryIntToBool<OpKind>>(Location, LHS->clone(), RHS->clone());
}
template <BinaryOpKindIntToBool OpKind>
std::unique_ptr<InterpreterValue> AstExprBinaryIntToBool<OpKind>::accept(AstVisitor& visitor) const {
    return visitor.visit(*this);
}
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>;
template class AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>;

template <BinaryOpKindBoolToBool OpKind>
AstExprBinaryBoolToBool<OpKind>::AstExprBinaryBoolToBool(const SourceLocation &loc,
                        std::unique_ptr<AstExpr> LHS,
                        std::unique_ptr<AstExpr> RHS) :
    AstExpr(loc), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
template <BinaryOpKindBoolToBool OpKind>
const AstExpr* AstExprBinaryBoolToBool<OpKind>::getLHS() const { return LHS.get(); }
template <BinaryOpKindBoolToBool OpKind>
const AstExpr* AstExprBinaryBoolToBool<OpKind>::getRHS() const { return RHS.get(); }
template <BinaryOpKindBoolToBool OpKind>
std::unique_ptr<AstExpr> AstExprBinaryBoolToBool<OpKind>::clone() const {
    return std::make_unique<AstExprBinaryBoolToBool<OpKind>>(Location, LHS->clone(), RHS->clone());
}
template <BinaryOpKindBoolToBool OpKind>
std::unique_ptr<InterpreterValue> AstExprBinaryBoolToBool<OpKind>::accept(AstVisitor& visitor) const {
    return visitor.visit(*this);
}
template class AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>;
template class AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>;

AstExprMatchPath::AstExprMatchPath(const SourceLocation &loc,
                    std::unique_ptr<AstExpr> guard,
                    std::unique_ptr<AstExpr> body) :
    Location(loc), Guard(std::move(guard)), Body(std::move(body)) {}
const SourceLocation& AstExprMatchPath::getLocation() const { return Location; }
const AstExpr* AstExprMatchPath::getGuard() const { return Guard.get(); }
const AstExpr* AstExprMatchPath::getBody() const { return Body.get(); }
std::unique_ptr<AstExprMatchPath> AstExprMatchPath::clone() const {
    return std::make_unique<AstExprMatchPath>(Location, Guard->clone(), Body->clone());
}

AstExprMatch::AstExprMatch(const SourceLocation &loc, std::vector<std::unique_ptr<AstExprMatchPath>> Paths) :
    AstExpr(loc), Paths(std::move(Paths)) {}
const std::vector<std::unique_ptr<AstExprMatchPath>>& AstExprMatch::getPaths() const { return Paths; }
std::unique_ptr<AstExpr> AstExprMatch::clone() const {
    std::vector<std::unique_ptr<AstExprMatchPath>> clonedPaths;
    for (const auto& path : Paths) {
        clonedPaths.push_back(path->clone());
    }
    return std::make_unique<AstExprMatch>(Location, std::move(clonedPaths));
}
std::unique_ptr<InterpreterValue> AstExprMatch::accept(AstVisitor& visitor) const {
    return visitor.visit(*this);
}