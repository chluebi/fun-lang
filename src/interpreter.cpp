#include <iostream>
#include <string>
#include <memory>
#include <vector>

class AstExpr {
public:
    virtual ~AstExpr() = default;
    virtual std::unique_ptr<AstExpr> eval() = 0;
};

class AstExprConst : public AstExpr {
  long Value;
public:
  AstExprConst(const long &Value) : Value(Value) {}
};

class AstExprVariable : public AstExpr {
  std::string Name;
public:
  AstExprVariable(const std::string &Name) : Name(Name) {}
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

    const AstExpr &getLHS() const { return *LHS; }
    const AstExpr &getRHS() const { return *RHS; }
};

class AstExprCall : public AstExpr {
  std::string Callee;
  std::vector<std::unique_ptr<AstExpr>> Args;

public:
  AstExprCall(const std::string &Callee,
              std::vector<std::unique_ptr<AstExpr>> Args)
    : Callee(Callee), Args(std::move(Args)) {}
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
};


int main(int argc, char *argv[]) {
    return 0;
}