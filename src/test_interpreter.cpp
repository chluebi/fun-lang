#include "interpreter.cpp" // Include your interpreter code
#include "tests.hpp"      // Include the test framework

// This function will test a single expression and return the evaluated value
long evaluateExpression(std::unique_ptr<AstExpr> expr) {
    Context context;
    // Add any necessary functions to the context if the expression requires them.
    // In this simple case, we'll just add the 'add' function.
    auto addFuncBody = std::make_unique<AstExprBinary<BinaryOpKind::Add>>(
        std::make_unique<AstExprVariable>("x"),
        std::make_unique<AstExprVariable>("y")
    );
    auto addFuncProto = std::make_unique<AstPrototype>("add", std::vector<std::string>{"x", "y"});
    auto addFunc = std::make_unique<AstFunction>(std::move(addFuncProto), std::move(addFuncBody));
    context.addFunction(std::move(addFunc));
    
    auto evaluated = expr->eval(context);
    AstExprConst* resultConst = dynamic_cast<AstExprConst*>(evaluated.get());
    if (resultConst) {
        return resultConst->getValue();
    }
    // Return a value that indicates failure if the expression doesn't evaluate to a constant.
    return -999999; 
}

TEST_CASE(ConstantEvaluation) {
    auto expr = std::make_unique<AstExprConst>(123);
    long result = evaluateExpression(std::move(expr));
    ASSERT_EQ(123, result);
}

TEST_CASE(Addition) {
    auto expr = std::make_unique<AstExprBinary<BinaryOpKind::Add>>(
        std::make_unique<AstExprConst>(10),
        std::make_unique<AstExprConst>(20)
    );
    long result = evaluateExpression(std::move(expr));
    ASSERT_EQ(30, result);
}

TEST_CASE(Subtraction) {
    auto expr = std::make_unique<AstExprBinary<BinaryOpKind::Sub>>(
        std::make_unique<AstExprConst>(50),
        std::make_unique<AstExprConst>(15)
    );
    long result = evaluateExpression(std::move(expr));
    ASSERT_EQ(35, result);
}

TEST_CASE(Multiplication) {
    auto expr = std::make_unique<AstExprBinary<BinaryOpKind::Mul>>(
        std::make_unique<AstExprConst>(7),
        std::make_unique<AstExprConst>(8)
    );
    long result = evaluateExpression(std::move(expr));
    ASSERT_EQ(56, result);
}

TEST_CASE(Division) {
    auto expr = std::make_unique<AstExprBinary<BinaryOpKind::Div>>(
        std::make_unique<AstExprConst>(100),
        std::make_unique<AstExprConst>(10)
    );
    long result = evaluateExpression(std::move(expr));
    ASSERT_EQ(10, result);
}

TEST_CASE(NestedBinaryExpressions) {
    auto inner_expr = std::make_unique<AstExprBinary<BinaryOpKind::Add>>(
        std::make_unique<AstExprConst>(5),
        std::make_unique<AstExprConst>(5)
    );
    auto outer_expr = std::make_unique<AstExprBinary<BinaryOpKind::Mul>>(
        std::move(inner_expr),
        std::make_unique<AstExprConst>(3)
    );
    long result = evaluateExpression(std::move(outer_expr));
    ASSERT_EQ(30, result); // (5 + 5) * 3 = 30
}

TEST_CASE(FunctionCall) {
    std::vector<std::unique_ptr<AstExpr>> args;
    args.push_back(std::make_unique<AstExprConst>(12));
    args.push_back(std::make_unique<AstExprConst>(34));
    auto call_expr = std::make_unique<AstExprCall>("add", std::move(args));
    long result = evaluateExpression(std::move(call_expr));
    ASSERT_EQ(46, result); // add(12, 34)
}

TEST_CASE(LetIn) {
    auto exprX = std::make_unique<AstExprBinary<BinaryOpKind::Add>>(
        std::make_unique<AstExprConst>(5),
        std::make_unique<AstExprConst>(10)
    );
    auto exprY = std::make_unique<AstExprBinary<BinaryOpKind::Sub>>(
        std::make_unique<AstExprConst>(10),
        std::make_unique<AstExprConst>(5)
    );

    std::vector<std::unique_ptr<AstExpr>> args;
    args.push_back(std::make_unique<AstExprVariable>("x"));
    args.push_back(std::make_unique<AstExprVariable>("y"));

    auto body = std::make_unique<AstExprCall>("add", std::move(args));
    auto expr = std::make_unique<AstExprLetIn>("x", std::move(exprX),
        std::make_unique<AstExprLetIn>("y", std::move(exprY),
                std::move(body)
            )
        );

    long result = evaluateExpression(std::move(expr));
    ASSERT_EQ(20, result); // let x := 5 + 10 in (let y := 10 - 5) in x + y
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}