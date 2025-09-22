#include "lexer.hpp"
#include "interpreter.hpp"

class Parser {
    Lexer TheLexer;

    std::unique_ptr<AstExpr> parsePrimary();
    std::unique_ptr<AstExpr> parseLetIn();
    std::unique_ptr<AstExpr> parseMatch();
    std::unique_ptr<AstExpr> parseCallOrVariable();
    std::unique_ptr<AstPrototype> parsePrototype();

    void nextToken() { TheLexer.nextToken(); }
    bool consume(TokenKind kind);
    void reportError(const std::string& msg);
    int getOpPrecedence(TokenKind kind);

public:
    Parser(const std::string& input) : TheLexer(input) {}
    const Token& get() const { return TheLexer.get(); }
    std::unique_ptr<AstExpr> parseExpression();
    std::unique_ptr<AstFunction> parseFunction();
    std::optional<std::unique_ptr<AstFunction>> parseTopLevelFunction();
};