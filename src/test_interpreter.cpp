#include <stdexcept>
#include <iostream>

#include "interpreter.hpp"
#include "interpreter_exception.hpp"
#include "tests.hpp"


long getLongResult(std::unique_ptr<InterpreterValue> result_val) {

    auto* result_long_ptr = dynamic_cast<InterpreterValueLong*>(result_val.get());
    if (!result_long_ptr) {
        std::cout << RED << "Assertion Failed: evaluation returned incorrect type (expected Long)" << RESET << std::endl;
        SimpleTestFramework::globalTestRunner.failTest();
        return -1; // Return a dummy value
    }
    return result_long_ptr->getValue();
}

bool getBoolResult(std::unique_ptr<InterpreterValue> result_val) {
    auto* result_bool_ptr = dynamic_cast<InterpreterValueBool*>(result_val.get());
    if (!result_bool_ptr) {
        std::cout << RED << "Assertion Failed: evaluation returned incorrect type (expected Bool)" << RESET << std::endl;
        SimpleTestFramework::globalTestRunner.failTest();
        return false;
    }
    return result_bool_ptr->getValue();
}


std::unique_ptr<InterpreterValue> evaluateExpression(std::unique_ptr<AstExpr> expr) {
    Context context;

    auto addFuncBody = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprVariable>(SourceLocation {0, 0}, "x"),
        std::make_unique<AstExprVariable>(SourceLocation {0, 0}, "y")
    );
    auto addFuncProto = std::make_unique<AstPrototype>(SourceLocation {0, 0}, "add", std::vector<AstArg>{
        AstArg {SourceLocation {0, 0}, "x"},
        AstArg {SourceLocation {0, 0}, "y"}
    });
    auto addFunc = std::make_unique<AstFunction>(SourceLocation {0, 0}, std::move(addFuncProto), std::move(addFuncBody));
    context.addFunction(std::move(addFunc));


    auto multiplyFuncBody = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprVariable>(SourceLocation {0, 0}, "x"),
        std::make_unique<AstExprVariable>(SourceLocation {0, 0}, "y")
    );
    auto multiplyFuncProto = std::make_unique<AstPrototype>(SourceLocation {0, 0}, "multiply", std::vector<AstArg>{
        AstArg {SourceLocation {0, 0}, "x"},
        AstArg {SourceLocation {0, 0}, "y"}
    });
    auto multiplyFunc = std::make_unique<AstFunction>(SourceLocation {0, 0}, std::move(multiplyFuncProto), std::move(multiplyFuncBody));
    context.addFunction(std::move(multiplyFunc));


    auto factorialProto = std::make_unique<AstPrototype>(SourceLocation {0, 0}, "factorial", std::vector<AstArg>{
        AstArg {SourceLocation {0, 0}, "n"}
    });
    auto factMatchGuard1 = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprVariable>(SourceLocation {0, 0}, "n"),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 0L)
    );
    auto factMatchBody1 = std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 1L);
    auto factMatchPath1 = std::make_unique<AstExprMatchPath>(SourceLocation {0, 0}, std::move(factMatchGuard1), std::move(factMatchBody1));
    auto factMatchGuard2 = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprVariable>(SourceLocation {0, 0}, "n"),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 0L)
    );
    auto factRecurseArg = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprVariable>(SourceLocation {0, 0}, "n"),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 1L)
    );
    std::vector<std::unique_ptr<AstExpr>> factCallArgs;
    factCallArgs.push_back(std::move(factRecurseArg));
    auto factRecurseCall = std::make_unique<AstExprCall>(SourceLocation {0, 0}, "factorial", std::move(factCallArgs));
    auto factMatchBody2 = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprVariable>(SourceLocation {0, 0}, "n"),
        std::move(factRecurseCall)
    );
    auto factMatchPath2 = std::make_unique<AstExprMatchPath>(SourceLocation {0, 0}, std::move(factMatchGuard2), std::move(factMatchBody2));
    std::vector<std::unique_ptr<AstExprMatchPath>> factPaths;
    factPaths.push_back(std::move(factMatchPath1));
    factPaths.push_back(std::move(factMatchPath2));
    auto factorialBody = std::make_unique<AstExprMatch>(SourceLocation {0, 0}, std::move(factPaths));
    auto factorialFunc = std::make_unique<AstFunction>(SourceLocation {0, 0}, std::move(factorialProto), std::move(factorialBody));
    context.addFunction(std::move(factorialFunc));

    Interpreter interpreter;
    std::unique_ptr<InterpreterValue> result;
    
    ASSERT_NOT_THROWS(result = interpreter.eval(*expr, context)); 
    
    return result;
}

TEST_CASE(ConstantEvaluation) {
    auto expr = std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 123L);
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(123L, result);
}

TEST_CASE(Addition) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 10L),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 20L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(30L, result);
}

TEST_CASE(Subtraction) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 50L),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 15L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(35L, result);
}

TEST_CASE(Multiplication) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 7L),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 8L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(56L, result);
}

TEST_CASE(Division) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 100L),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 10L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(10L, result);
}

TEST_CASE(NestedBinaryExpressions) {
    auto inner_expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 5L),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 5L)
    );
    auto outer_expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>>(
        SourceLocation {0, 0},
        std::move(inner_expr),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 3L)
    );
    long result = getLongResult(evaluateExpression(std::move(outer_expr)));
    ASSERT_EQ(30L, result); // (5 + 5) * 3 = 30
}


TEST_CASE(FunctionCall) {
    std::vector<std::unique_ptr<AstExpr>> args;
    args.push_back(std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 12L));
    args.push_back(std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 34L));
    auto call_expr = std::make_unique<AstExprCall>(SourceLocation {0, 0}, "add", std::move(args));
    long result = getLongResult(evaluateExpression(std::move(call_expr)));
    ASSERT_EQ(46L, result); // add(12, 34)
}

TEST_CASE(LetIn) {
    auto exprX = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 5L),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 10L)
    );
    auto exprY = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 10L),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 5L)
    );

    std::vector<std::unique_ptr<AstExpr>> args;
    args.push_back(std::make_unique<AstExprVariable>(SourceLocation {0, 0}, "x"));
    args.push_back(std::make_unique<AstExprVariable>(SourceLocation {0, 0}, "y"));

    auto body = std::make_unique<AstExprCall>(SourceLocation {0, 0}, "add", std::move(args));
    auto expr = std::make_unique<AstExprLetIn>(SourceLocation {0, 0}, "x", std::move(exprX),
        std::make_unique<AstExprLetIn>(SourceLocation {0, 0}, "y", std::move(exprY),
                std::move(body)
            )
        );

    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(20L, result); // let x := 5 + 10 in (let y := 10 - 5 in add(x, y))
}

TEST_CASE(Equality) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 5L),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 5L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(true, result);
}

TEST_CASE(EqualityFalse) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 5L),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 6L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(NotEqual) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 5L),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 10L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(true, result);
}

// Expected Failures

TEST_CASE(DivisionByZero) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 100L),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 0L)
    );
    Interpreter interpreter;
    ASSERT_THROWS(interpreter.eval(*expr, Context()), DivisionByZeroException);
}

TEST_CASE(FunctionCallWithWrongNumberOfArguments) {
    std::vector<std::unique_ptr<AstExpr>> args;
    args.push_back(std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 10L)); // missing one argument
    auto call_expr = std::make_unique<AstExprCall>(SourceLocation {0, 0}, "add", std::move(args));
    Interpreter interpreter;
    Context context;
    // Set up 'add' function again for local context testing
    auto addFuncBody = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>>(SourceLocation {0, 0}, std::make_unique<AstExprVariable>(SourceLocation {0, 0}, "x"), std::make_unique<AstExprVariable>(SourceLocation {0, 0}, "y"));
    auto addFuncProto = std::make_unique<AstPrototype>(SourceLocation {0, 0}, "add", std::vector<AstArg>{AstArg {SourceLocation {0, 0}, "x"}, AstArg {SourceLocation {0, 0}, "y"}});
    auto addFunc = std::make_unique<AstFunction>(SourceLocation {0, 0}, std::move(addFuncProto), std::move(addFuncBody));
    context.addFunction(std::move(addFunc));

    ASSERT_THROWS(interpreter.eval(*call_expr, context), ArityMismatchException);
}

TEST_CASE(UnknownVariable) {
    auto expr = std::make_unique<AstExprVariable>(SourceLocation {0, 0}, "unknown_var");
    Interpreter interpreter;
    ASSERT_THROWS(interpreter.eval(*expr, Context()), UndefinedVariableException);
}

TEST_CASE(UnknownFunction) {
    std::vector<std::unique_ptr<AstExpr>> args;
    args.push_back(std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 1L));
    args.push_back(std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 2L));
    auto expr = std::make_unique<AstExprCall>(SourceLocation {0, 0}, "unknown_func", std::move(args));
    Interpreter interpreter;
    ASSERT_THROWS(interpreter.eval(*expr, Context()), UndefinedFunctionException);
}

TEST_CASE(LetInVariableNotFound) {
    auto body = std::make_unique<AstExprVariable>(SourceLocation {0, 0}, "z");
    
    // let y := 10L in z
    auto expr = std::make_unique<AstExprLetIn>(SourceLocation {0, 0}, "y",
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 10L),
        std::move(body)
    );
    
    Interpreter interpreter;
    ASSERT_THROWS(interpreter.eval(*expr, Context()), UndefinedVariableException);
}


// --- Boolean Operations (False results) ---

TEST_CASE(Equality_False) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 5L),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 10L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(NotEqual_False) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>>(
        SourceLocation {0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 5L),
        std::make_unique<AstExprConstLong>(SourceLocation {0, 0}, 5L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(LessThan_False) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 5L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(LessThanOrEqual_False) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 5L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(GreaterThan_False) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 5L),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(GreaterThanOrEqual_False) {
    auto expr = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 5L),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(LargeNumberAddition) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 9876543210L),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 1234567890L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(11111111100L, result);
}

TEST_CASE(NegativeNumberSubtraction) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, -100L),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, -50L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(-50L, result);
}

TEST_CASE(MixedSignMultiplication) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, -12L),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(-120L, result);
}

TEST_CASE(NegativeNumberDivision) {
    auto expr = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, -200L),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 20L)
    );
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(-10L, result);
}

TEST_CASE(LetInNestedFunctions) {
    auto exprX = std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 5L);
    auto exprY = std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L);

    std::vector<std::unique_ptr<AstExpr>> argsAdd;
    argsAdd.push_back(std::make_unique<AstExprVariable>(SourceLocation{0, 0}, "x"));
    argsAdd.push_back(std::make_unique<AstExprVariable>(SourceLocation{0, 0}, "y"));
    auto addCall = std::make_unique<AstExprCall>(SourceLocation{0, 0}, "add", std::move(argsAdd));

    std::vector<std::unique_ptr<AstExpr>> argsMult;
    argsMult.push_back(std::move(addCall));
    argsMult.push_back(std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 2L));
    auto multCall = std::make_unique<AstExprCall>(SourceLocation{0, 0}, "multiply", std::move(argsMult));

    auto expr = std::make_unique<AstExprLetIn>(SourceLocation{0, 0}, "x", std::move(exprX),
        std::make_unique<AstExprLetIn>(SourceLocation{0, 0}, "y", std::move(exprY),
                std::move(multCall)
            )
        );

    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(30L, result); // let x := 5 in (let y := 10 in multiply(add(x,y), 2))
}

TEST_CASE(BooleanAnd_True) {
    auto expr = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, true),
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, true)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(true, result);
}

TEST_CASE(BooleanAnd_False) {
    auto expr = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, true),
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, false)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

TEST_CASE(BooleanOr_True) {
    auto expr = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, false),
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, true)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(true, result);
}

TEST_CASE(BooleanOr_False) {
    auto expr = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, false),
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, false)
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(false, result);
}

// New test for mixed boolean and integer expressions
TEST_CASE(MixedBooleanAndInt) {
    auto expr = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>>(
            SourceLocation{0, 0},
            std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L),
            std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 5L)
        ),
        std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>>(
            SourceLocation{0, 0},
            std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 20L),
            std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 20L)
        )
    );
    bool result = getBoolResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(true, result); // (10 > 5) && (20 == 20)
}

// Expected failures for boolean operations with incorrect types
TEST_CASE(BooleanAnd_TypeError) {
    auto expr = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, true),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L) // Incorrect type
    );
    Interpreter interpreter;
    ASSERT_THROWS(interpreter.eval(*expr, Context()), TypeMismatchException);
}

TEST_CASE(BooleanOr_TypeError) {
    auto expr = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L), // Incorrect type
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, true)
    );
    Interpreter interpreter;
    ASSERT_THROWS(interpreter.eval(*expr, Context()), TypeMismatchException);
}

TEST_CASE(Match_FirstPathTrue) {
    std::vector<std::unique_ptr<AstExprMatchPath>> paths;
    paths.push_back(std::make_unique<AstExprMatchPath>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, true),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L)
    ));
    paths.push_back(std::make_unique<AstExprMatchPath>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, false),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 20L)
    ));
    auto expr = std::make_unique<AstExprMatch>(SourceLocation{0, 0}, std::move(paths));
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(10L, result);
}

TEST_CASE(Match_SecondPathTrue) {
    std::vector<std::unique_ptr<AstExprMatchPath>> paths;
    paths.push_back(std::make_unique<AstExprMatchPath>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, false),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L)
    ));
    paths.push_back(std::make_unique<AstExprMatchPath>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, true),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 20L)
    ));
    auto expr = std::make_unique<AstExprMatch>(SourceLocation{0, 0}, std::move(paths));
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(20L, result);
}

TEST_CASE(Match_NoPathTrue) {
    std::vector<std::unique_ptr<AstExprMatchPath>> paths;
    paths.push_back(std::make_unique<AstExprMatchPath>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, false),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L)
    ));
    paths.push_back(std::make_unique<AstExprMatchPath>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, false),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 20L)
    ));
    auto expr = std::make_unique<AstExprMatch>(SourceLocation{0, 0}, std::move(paths));
    Interpreter interpreter;
    ASSERT_THROWS(interpreter.eval(*expr, Context()), NoMatchFoundException);
}


TEST_CASE(Match_WithLetInAndCalls) {
    // let x := 15L
    auto letExpr = std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 15L);
    
    // x > 10L
    auto guard1 = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprVariable>(SourceLocation{0, 0}, "x"),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L)
    );
    
    // x + 5L
    auto body1 = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprVariable>(SourceLocation{0, 0}, "x"),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 5L)
    );
    
    auto path1 = std::make_unique<AstExprMatchPath>(SourceLocation{0, 0}, std::move(guard1), std::move(body1));

    // true
    auto guard2 = std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, true);

    // 0L
    auto body2 = std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 0L);

    auto path2 = std::make_unique<AstExprMatchPath>(SourceLocation{0, 0}, std::move(guard2), std::move(body2));
    
    std::vector<std::unique_ptr<AstExprMatchPath>> paths;
    paths.push_back(std::move(path1));
    paths.push_back(std::move(path2));

    auto matchExpr = std::make_unique<AstExprMatch>(SourceLocation{0, 0}, std::move(paths));
    
    auto expr = std::make_unique<AstExprLetIn>(SourceLocation{0, 0}, "x", std::move(letExpr), std::move(matchExpr));
    
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(20L, result);
}

TEST_CASE(Match_NestedExpressions) {
    std::vector<std::unique_ptr<AstExprMatchPath>> paths;
    
    // Guard: (5L == 5L)
    auto guard1 = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 5L),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 5L)
    );
    
    // Body: (10L * 2L)
    auto body1 = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>>(
        SourceLocation{0, 0},
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L),
        std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 2L)
    );
    
    paths.push_back(std::make_unique<AstExprMatchPath>(SourceLocation{0, 0}, std::move(guard1), std::move(body1)));

    // Guard: true
    auto guard2 = std::make_unique<AstExprConstBool>(SourceLocation{0, 0}, true);
    
    // Body: 100L
    auto body2 = std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 100L);
    
    paths.push_back(std::make_unique<AstExprMatchPath>(SourceLocation{0, 0}, std::move(guard2), std::move(body2)));

    auto expr = std::make_unique<AstExprMatch>(SourceLocation{0, 0}, std::move(paths));
    long result = getLongResult(evaluateExpression(std::move(expr)));
    ASSERT_EQ(20L, result);
}


TEST_CASE(Match_GuardTypeError) {
    std::vector<std::unique_ptr<AstExprMatchPath>> paths;
    
    // Guard: 10L (should be bool)
    auto guard = std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L);
    
    // Body: 10L
    auto body = std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 10L);
    
    paths.push_back(std::make_unique<AstExprMatchPath>(SourceLocation{0, 0}, std::move(guard), std::move(body)));
    
    auto expr = std::make_unique<AstExprMatch>(SourceLocation{0, 0}, std::move(paths));
    Interpreter interpreter;
    ASSERT_THROWS(interpreter.eval(*expr, Context()), TypeMismatchException);
}

TEST_CASE(Factorial) {
    std::vector<std::unique_ptr<AstExpr>> args;
    args.push_back(std::make_unique<AstExprConstLong>(SourceLocation{0, 0}, 5L));
    auto call_expr = std::make_unique<AstExprCall>(SourceLocation{0, 0}, "factorial", std::move(args));
    long result = getLongResult(evaluateExpression(std::move(call_expr)));
    ASSERT_EQ(120L, result);
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}