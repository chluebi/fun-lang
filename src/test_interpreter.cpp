#include "interpreter.cpp"
#include "tests.hpp"

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

// This helper function checks if the evaluation was successful and returns the
// correct type, then extracts the bool value. It fails the test if not.
bool getBoolResult(std::unique_ptr<InterpreterValue> result_val) {
    if (!result_val) {
        std::cout << RED << "Assertion Failed: evaluation returned nullptr" << RESET << std::endl;
        SimpleTestFramework::globalTestRunner.failTest();
        return false;
    }

    auto* result_bool_ptr = dynamic_cast<InterpreterValueBool*>(result_val.get());
    if (!result_bool_ptr) {
        std::cout << RED << "Assertion Failed: evaluation returned incorrect type" << RESET << std::endl;
        SimpleTestFramework::globalTestRunner.failTest();
        return false;
    }
    return result_bool_ptr->getValue();
}


// This function will test a single expression and return the evaluated value
std::unique_ptr<InterpreterValue> evaluateExpression(std::unique_ptr<AstExpr> expr) {
    Context context;


    auto addFuncBody = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>>(
        std::make_unique<AstExprVariable>("x"),
        std::make_unique<AstExprVariable>("y")
    );
    auto addFuncProto = std::make_unique<AstPrototype>("add", std::vector<std::string>{"x", "y"});
    auto addFunc = std::make_unique<AstFunction>(std::move(addFuncProto), std::move(addFuncBody));
    context.addFunction(std::move(addFunc));


    auto multiplyFuncBody = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>>(
        std::make_unique<AstExprVariable>("x"),
        std::make_unique<AstExprVariable>("y")
    );
    auto multiplyFuncProto = std::make_unique<AstPrototype>("multiply", std::vector<std::string>{"x", "y"});
    auto multiplyFunc = std::make_unique<AstFunction>(std::move(multiplyFuncProto), std::move(multiplyFuncBody));
    context.addFunction(std::move(multiplyFunc));


    // Factorial function
    auto factorialProto = std::make_unique<AstPrototype>("factorial", std::vector<std::string>{"n"});
    
    // Base case: n == 0
    auto factMatchGuard1 = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>>(
        std::make_unique<AstExprVariable>("n"),
        std::make_unique<AstExprConstLong>(0L)
    );
    auto factMatchBody1 = std::make_unique<AstExprConstLong>(1L);
    auto factMatchPath1 = std::make_unique<AstExprMatchPath>(std::move(factMatchGuard1), std::move(factMatchBody1));

    // Recursive case: n > 0
    auto factMatchGuard2 = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>>(
        std::make_unique<AstExprVariable>("n"),
        std::make_unique<AstExprConstLong>(0L)
    );
    
    // Arguments for recursive call: n-1
    auto factRecurseArg = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>>(
        std::make_unique<AstExprVariable>("n"),
        std::make_unique<AstExprConstLong>(1L)
    );
    
    // Recursive call: factorial(n - 1)
    std::vector<std::unique_ptr<AstExpr>> factCallArgs;
    factCallArgs.push_back(std::move(factRecurseArg));
    auto factRecurseCall = std::make_unique<AstExprCall>("factorial", std::move(factCallArgs));
    
    // Body: n * factorial(n - 1)
    auto factMatchBody2 = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>>(
        std::make_unique<AstExprVariable>("n"),
        std::move(factRecurseCall)
    );
    
    auto factMatchPath2 = std::make_unique<AstExprMatchPath>(std::move(factMatchGuard2), std::move(factMatchBody2));

    std::vector<std::unique_ptr<AstExprMatchPath>> factPaths;
    factPaths.push_back(std::move(factMatchPath1));
    factPaths.push_back(std::move(factMatchPath2));
    
    auto factorialBody = std::make_unique<AstExprMatch>(std::move(factPaths));
    auto factorialFunc = std::make_unique<AstFunction>(std::move(factorialProto), std::move(factorialBody));
    context.addFunction(std::move(factorialFunc));

    
    return expr->eval(context);
}

TEST_CASE(ConstantEvaluation) {
    auto expr = std::make_unique<AstExprConstLong>(123L);
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(123L, result);
}

TEST_CASE(Addition) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>>(
        std::make_unique<AstExprConstLong>(10L),
        std::make_unique<AstExprConstLong>(20L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(30L, result);
}

TEST_CASE(Subtraction) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>>(
        std::make_unique<AstExprConstLong>(50L),
        std::make_unique<AstExprConstLong>(15L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(35L, result);
}

TEST_CASE(Multiplication) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>>(
        std::make_unique<AstExprConstLong>(7L),
        std::make_unique<AstExprConstLong>(8L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(56L, result);
}

TEST_CASE(Division) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>>(
        std::make_unique<AstExprConstLong>(100L),
        std::make_unique<AstExprConstLong>(10L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(10L, result);
}

TEST_CASE(NestedBinaryExpressions) {
    auto inner_expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>>(
        std::make_unique<AstExprConstLong>(5L),
        std::make_unique<AstExprConstLong>(5L)
    );
    auto outer_expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>>(
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
    auto exprX = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>>(
        std::make_unique<AstExprConstLong>(5L),
        std::make_unique<AstExprConstLong>(10L)
    );
    auto exprY = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>>(
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

TEST_CASE(Equality) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>>(
        std::make_unique<AstExprConstLong>(5L),
        std::make_unique<AstExprConstLong>(5L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(true, result);
}

TEST_CASE(EqualityFalse) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>>(
        std::make_unique<AstExprConstLong>(5L),
        std::make_unique<AstExprConstLong>(6L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(NotEqual) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>>(
        std::make_unique<AstExprConstLong>(5L),
        std::make_unique<AstExprConstLong>(10L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(true, result);
}

// Expected Failures

TEST_CASE(DivisionByZero) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>>(
        std::make_unique<AstExprConstLong>(100L),
        std::make_unique<AstExprConstLong>(0L)
    );
    std::unique_ptr<InterpreterValue> result = evaluateExpression(std::move(expr));
    ASSERT_EQ(nullptr, result.get());
}

TEST_CASE(FunctionCallWithWrongNumberOfArguments) {
    std::vector<std::unique_ptr<AstExpr>> args;
    args.push_back(std::make_unique<AstExprConstLong>(10L));
    auto call_expr = std::make_unique<AstExprCall>("add", std::move(args));
    std::unique_ptr<InterpreterValue> result = evaluateExpression(std::move(call_expr));
    ASSERT_EQ(nullptr, result.get());
}

TEST_CASE(UnknownVariable) {
    auto expr = std::make_unique<AstExprVariable>("unknown_var");
    std::unique_ptr<InterpreterValue> result = evaluateExpression(std::move(expr));
    ASSERT_EQ(nullptr, result.get());
}

TEST_CASE(UnknownFunction) {
    std::vector<std::unique_ptr<AstExpr>> args;
    args.push_back(std::make_unique<AstExprConstLong>(1L));
    args.push_back(std::make_unique<AstExprConstLong>(2L));
    auto expr = std::make_unique<AstExprCall>("unknown_func", std::move(args));
    std::unique_ptr<InterpreterValue> result = evaluateExpression(std::move(expr));
    ASSERT_EQ(nullptr, result.get());
}

TEST_CASE(LetInVariableNotFound) {
    auto body = std::make_unique<AstExprVariable>("x");
    auto expr = std::make_unique<AstExprLetIn>("y",
        std::make_unique<AstExprConstLong>(10L),
        std::move(body)
    );
    std::unique_ptr<InterpreterValue> result = evaluateExpression(std::move(expr));
    ASSERT_EQ(nullptr, result.get()); // 'x' is not defined
}

// --- Boolean Operations (False results) ---

TEST_CASE(Equality_False) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>>(
        std::make_unique<AstExprConstLong>(5L),
        std::make_unique<AstExprConstLong>(10L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(NotEqual_False) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>>(
        std::make_unique<AstExprConstLong>(5L),
        std::make_unique<AstExprConstLong>(5L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(LessThan_False) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>>(
        std::make_unique<AstExprConstLong>(10L),
        std::make_unique<AstExprConstLong>(5L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(LessThanOrEqual_False) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>>(
        std::make_unique<AstExprConstLong>(10L),
        std::make_unique<AstExprConstLong>(5L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(GreaterThan_False) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>>(
        std::make_unique<AstExprConstLong>(5L),
        std::make_unique<AstExprConstLong>(10L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(GreaterThanOrEqual_False) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>>(
        std::make_unique<AstExprConstLong>(5L),
        std::make_unique<AstExprConstLong>(10L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}


TEST_CASE(LargeNumberAddition) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>>(
        std::make_unique<AstExprConstLong>(9876543210L),
        std::make_unique<AstExprConstLong>(1234567890L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(11111111100L, result);
}

TEST_CASE(NegativeNumberSubtraction) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>>(
        std::make_unique<AstExprConstLong>(-100L),
        std::make_unique<AstExprConstLong>(-50L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(-50L, result);
}

TEST_CASE(MixedSignMultiplication) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>>(
        std::make_unique<AstExprConstLong>(-12L),
        std::make_unique<AstExprConstLong>(10L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(-120L, result);
}

TEST_CASE(NegativeNumberDivision) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>>(
        std::make_unique<AstExprConstLong>(-200L),
        std::make_unique<AstExprConstLong>(20L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(-10L, result);
}

TEST_CASE(LetInNestedFunctions) {
    auto exprX = std::make_unique<AstExprConstLong>(5L);
    auto exprY = std::make_unique<AstExprConstLong>(10L);

    std::vector<std::unique_ptr<AstExpr>> argsAdd;
    argsAdd.push_back(std::make_unique<AstExprVariable>("x"));
    argsAdd.push_back(std::make_unique<AstExprVariable>("y"));
    auto addCall = std::make_unique<AstExprCall>("add", std::move(argsAdd));

    std::vector<std::unique_ptr<AstExpr>> argsMult;
    argsMult.push_back(std::move(addCall));
    argsMult.push_back(std::make_unique<AstExprConstLong>(2L));
    auto multCall = std::make_unique<AstExprCall>("multiply", std::move(argsMult));

    auto expr = std::make_unique<AstExprLetIn>("x", std::move(exprX),
        std::make_unique<AstExprLetIn>("y", std::move(exprY),
                std::move(multCall)
            )
        );

    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(30L, result); // let x := 5 in (let y := 10 in multiply(add(x,y), 2))
}

TEST_CASE(BooleanAnd_True) {
    auto expr = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>>(
        std::make_unique<AstExprConstBool>(true),
        std::make_unique<AstExprConstBool>(true)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(true, result);
}

TEST_CASE(BooleanAnd_False) {
    auto expr = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>>(
        std::make_unique<AstExprConstBool>(true),
        std::make_unique<AstExprConstBool>(false)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(BooleanOr_True) {
    auto expr = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>>(
        std::make_unique<AstExprConstBool>(false),
        std::make_unique<AstExprConstBool>(true)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(true, result);
}

TEST_CASE(BooleanOr_False) {
    auto expr = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>>(
        std::make_unique<AstExprConstBool>(false),
        std::make_unique<AstExprConstBool>(false)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

// New test for mixed boolean and integer expressions
TEST_CASE(MixedBooleanAndInt) {
    auto expr = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>>(
        std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>>(
            std::make_unique<AstExprConstLong>(10L),
            std::make_unique<AstExprConstLong>(5L)
        ),
        std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>>(
            std::make_unique<AstExprConstLong>(20L),
            std::make_unique<AstExprConstLong>(20L)
        )
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(true, result); // (10 > 5) && (20 == 20)
}

// Expected failures for boolean operations with incorrect types
TEST_CASE(BooleanAnd_TypeError) {
    auto expr = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>>(
        std::make_unique<AstExprConstBool>(true),
        std::make_unique<AstExprConstLong>(10L)
    );
    std::unique_ptr<InterpreterValue> result = evaluateExpression(std::move(expr));
    ASSERT_EQ(nullptr, result.get());
}

TEST_CASE(BooleanOr_TypeError) {
    auto expr = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>>(
        std::make_unique<AstExprConstLong>(10L),
        std::make_unique<AstExprConstBool>(true)
    );
    std::unique_ptr<InterpreterValue> result = evaluateExpression(std::move(expr));
    ASSERT_EQ(nullptr, result.get());
}

TEST_CASE(Match_FirstPathTrue) {
    std::vector<std::unique_ptr<AstExprMatchPath>> paths;
    paths.push_back(std::make_unique<AstExprMatchPath>(
        std::make_unique<AstExprConstBool>(true),
        std::make_unique<AstExprConstLong>(10L)
    ));
    paths.push_back(std::make_unique<AstExprMatchPath>(
        std::make_unique<AstExprConstBool>(false),
        std::make_unique<AstExprConstLong>(20L)
    ));
    auto expr = std::make_unique<AstExprMatch>(std::move(paths));
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(10L, result);
}

TEST_CASE(Match_SecondPathTrue) {
    std::vector<std::unique_ptr<AstExprMatchPath>> paths;
    paths.push_back(std::make_unique<AstExprMatchPath>(
        std::make_unique<AstExprConstBool>(false),
        std::make_unique<AstExprConstLong>(10L)
    ));
    paths.push_back(std::make_unique<AstExprMatchPath>(
        std::make_unique<AstExprConstBool>(true),
        std::make_unique<AstExprConstLong>(20L)
    ));
    auto expr = std::make_unique<AstExprMatch>(std::move(paths));
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(20L, result);
}

TEST_CASE(Match_NoPathTrue) {
    std::vector<std::unique_ptr<AstExprMatchPath>> paths;
    paths.push_back(std::make_unique<AstExprMatchPath>(
        std::make_unique<AstExprConstBool>(false),
        std::make_unique<AstExprConstLong>(10L)
    ));
    paths.push_back(std::make_unique<AstExprMatchPath>(
        std::make_unique<AstExprConstBool>(false),
        std::make_unique<AstExprConstLong>(20L)
    ));
    auto expr = std::make_unique<AstExprMatch>(std::move(paths));
    std::unique_ptr<InterpreterValue> result = evaluateExpression(std::move(expr));
    ASSERT_EQ(nullptr, result.get());
}

TEST_CASE(Match_WithLetInAndCalls) {
    // let x := 15L
    auto letExpr = std::make_unique<AstExprConstLong>(15L);
    
    // x > 10L
    auto guard1 = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>>(
        std::make_unique<AstExprVariable>("x"),
        std::make_unique<AstExprConstLong>(10L)
    );
    
    // x + 5L
    auto body1 = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>>(
        std::make_unique<AstExprVariable>("x"),
        std::make_unique<AstExprConstLong>(5L)
    );
    
    auto path1 = std::make_unique<AstExprMatchPath>(std::move(guard1), std::move(body1));

    // true
    auto guard2 = std::make_unique<AstExprConstBool>(true);

    // 0L
    auto body2 = std::make_unique<AstExprConstLong>(0L);

    auto path2 = std::make_unique<AstExprMatchPath>(std::move(guard2), std::move(body2));
    
    std::vector<std::unique_ptr<AstExprMatchPath>> paths;
    paths.push_back(std::move(path1));
    paths.push_back(std::move(path2));

    auto matchExpr = std::make_unique<AstExprMatch>(std::move(paths));
    
    auto expr = std::make_unique<AstExprLetIn>("x", std::move(letExpr), std::move(matchExpr));
    
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(20L, result);
}

TEST_CASE(Match_NestedExpressions) {
    std::vector<std::unique_ptr<AstExprMatchPath>> paths;
    
    // Guard: (5L == 5L)
    auto guard1 = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>>(
        std::make_unique<AstExprConstLong>(5L),
        std::make_unique<AstExprConstLong>(5L)
    );
    
    // Body: (10L * 2L)
    auto body1 = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>>(
        std::make_unique<AstExprConstLong>(10L),
        std::make_unique<AstExprConstLong>(2L)
    );
    
    paths.push_back(std::make_unique<AstExprMatchPath>(std::move(guard1), std::move(body1)));

    // Guard: true
    auto guard2 = std::make_unique<AstExprConstBool>(true);
    
    // Body: 100L
    auto body2 = std::make_unique<AstExprConstLong>(100L);
    
    paths.push_back(std::make_unique<AstExprMatchPath>(std::move(guard2), std::move(body2)));

    auto expr = std::make_unique<AstExprMatch>(std::move(paths));
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(20L, result);
}

TEST_CASE(Match_GuardTypeError) {
    std::vector<std::unique_ptr<AstExprMatchPath>> paths;
    
    // Guard: 10L (should be bool)
    auto guard = std::make_unique<AstExprConstLong>(10L);
    
    // Body: 10L
    auto body = std::make_unique<AstExprConstLong>(10L);
    
    paths.push_back(std::make_unique<AstExprMatchPath>(std::move(guard), std::move(body)));
    
    auto expr = std::make_unique<AstExprMatch>(std::move(paths));
    std::unique_ptr<InterpreterValue> result = evaluateExpression(std::move(expr));
    ASSERT_EQ(nullptr, result.get());
}

TEST_CASE(Factorial) {
    std::vector<std::unique_ptr<AstExpr>> args;
    args.push_back(std::make_unique<AstExprConstLong>(5L));
    auto call_expr = std::make_unique<AstExprCall>("factorial", std::move(args));
    long result = getLongResult(evaluateExpression(std::move(call_expr)));
    ASSERT_EQ(120L, result);
}



int main() {
    RUN_ALL_TESTS();
    return 0;
}