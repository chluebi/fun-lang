#include "interpreter.cpp" // Include your interpreter code
#include "tests.hpp"      // Include the test framework

// This helper function checks if the evaluation was successful and returns the
// correct type, then extracts the long value. It fails the test if not.
long getLongResult(std::unique_ptr<InterpreterValue> result_val) {
    if (!result_val) {
        std::cout << RED << "Assertion Failed: evaluation returned nullptr" << RESET << std::endl;
        SimpleTestFramework::globalTestRunner.failTest();
        return -1; // Return a dummy value
    }

    auto* result_long_ptr = dynamic_cast<InterpreterValueLong*>(result_val.get());
    if (!result_long_ptr) {
        std::cout << RED << "Assertion Failed: evaluation returned incorrect type" << RESET << std::endl;
        SimpleTestFramework::globalTestRunner.failTest();
        return -1; // Return a dummy value
    }
    return result_long_ptr->getValue();
}


// This function will test a single expression and return the evaluated value
std::unique_ptr<InterpreterValue> evaluateExpression(std::unique_ptr<AstExpr> expr) {
    Context context;

    auto addFuncBody = std::make_unique<AstExprBinary<BinaryOpKind::Add>>(
        std::make_unique<AstExprVariable>("x"),
        std::make_unique<AstExprVariable>("y")
    );
    auto addFuncProto = std::make_unique<AstPrototype>("add", std::vector<std::string>{"x", "y"});
    auto addFunc = std::make_unique<AstFunction>(std::move(addFuncProto), std::move(addFuncBody));
    context.addFunction(std::move(addFunc));
    
    return expr->eval(context);
}

TEST_CASE(ConstantEvaluation) {
    auto expr = std::make_unique<AstExprConstLong>(123L);
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(123L, result);
}

TEST_CASE(Addition) {
    auto expr = std::make_unique<AstExprBinary<BinaryOpKind::Add>>(
        std::make_unique<AstExprConstLong>(10L),
        std::make_unique<AstExprConstLong>(20L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(30L, result);
}

TEST_CASE(Subtraction) {
    auto expr = std::make_unique<AstExprBinary<BinaryOpKind::Sub>>(
        std::make_unique<AstExprConstLong>(50L),
        std::make_unique<AstExprConstLong>(15L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(35L, result);
}

TEST_CASE(Multiplication) {
    auto expr = std::make_unique<AstExprBinary<BinaryOpKind::Mul>>(
        std::make_unique<AstExprConstLong>(7L),
        std::make_unique<AstExprConstLong>(8L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(56L, result);
}

TEST_CASE(Division) {
    auto expr = std::make_unique<AstExprBinary<BinaryOpKind::Div>>(
        std::make_unique<AstExprConstLong>(100L),
        std::make_unique<AstExprConstLong>(10L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(10L, result);
}

TEST_CASE(NestedBinaryExpressions) {
    auto inner_expr = std::make_unique<AstExprBinary<BinaryOpKind::Add>>(
        std::make_unique<AstExprConstLong>(5L),
        std::make_unique<AstExprConstLong>(5L)
    );
    auto outer_expr = std::make_unique<AstExprBinary<BinaryOpKind::Mul>>(
        std::move(inner_expr),
        std::make_unique<AstExprConstLong>(3L)
    );
    long result = getLongResult(evaluateExpression(std::move(outer_expr)));
    ASSERT_EQ(30L, result); // (5 + 5) * 3 = 30
}

TEST_CASE(FunctionCall) {
    std::vector<std::unique_ptr<AstExpr>> args;
    args.push_back(std::make_unique<AstExprConstLong>(12L));
    args.push_back(std::make_unique<AstExprConstLong>(34L));
    auto call_expr = std::make_unique<AstExprCall>("add", std::move(args));
    long result = getLongResult(evaluateExpression(std::move(call_expr)));
    ASSERT_EQ(46L, result); // add(12, 34)
}

TEST_CASE(LetIn) {
    auto exprX = std::make_unique<AstExprBinary<BinaryOpKind::Add>>(
        std::make_unique<AstExprConstLong>(5L),
        std::make_unique<AstExprConstLong>(10L)
    );
    auto exprY = std::make_unique<AstExprBinary<BinaryOpKind::Sub>>(
        std::make_unique<AstExprConstLong>(10L),
        std::make_unique<AstExprConstLong>(5L)
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

    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(20L, result); // let x := 5 + 10 in (let y := 10 - 5 in add(x, y))
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}