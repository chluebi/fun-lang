#include "parser.hpp"
#include "source_location.hpp"
#include <functional>

bool Parser::consume(TokenKind kind) {
    if (get().Kind == kind) {
        nextToken();
        return true;
    }
    return false;
}

void Parser::reportError(const std::string& msg) {
    std::cerr << "Parser error: " << msg << " at token " << get().Text << std::endl;
}

int Parser::getOpPrecedence(TokenKind kind) {
    switch (kind) {
        case TokenKind::Or: return 10;
        case TokenKind::And: return 20;
        case TokenKind::Eq: case TokenKind::Neq: return 30;
        case TokenKind::Lt: case TokenKind::Leq: case TokenKind::Gt: case TokenKind::Geq: return 40;
        case TokenKind::Add: case TokenKind::Sub: return 50;
        case TokenKind::Mul: case TokenKind::Div: return 60;
        default: return -1;
    }
}

std::unique_ptr<AstExpr> Parser::parseExpression() {
    auto LHS = parsePrimary();
    if (!LHS) return nullptr;

    std::function<std::unique_ptr<AstExpr>(int, std::unique_ptr<AstExpr>)> 
    parseBinOpRHS = [&](int exprPrec, std::unique_ptr<AstExpr> LHS) {
        while (true) {
            int tokenPrec = getOpPrecedence(get().Kind);
            if (tokenPrec < exprPrec) return LHS;

            TokenKind binOp = get().Kind;
            nextToken();

            auto RHS = parsePrimary();
            if (!RHS) return std::unique_ptr<AstExpr>(nullptr);

            int nextPrec = getOpPrecedence(get().Kind);
            if (tokenPrec < nextPrec) {
                RHS = parseBinOpRHS(tokenPrec + 1, std::move(RHS));
                if (!RHS) return std::unique_ptr<AstExpr>(nullptr);
            }

            // Create the binary expression node based on the operator kind
            switch (binOp) {
                case TokenKind::Add:
                    LHS = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Add>>(
                        mergeLocations(LHS->getLocation(), RHS->getLocation()),
                        std::move(LHS),
                        std::move(RHS)
                    );
                    break;
                case TokenKind::Sub:
                    LHS = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Sub>>(
                        mergeLocations(LHS->getLocation(), RHS->getLocation()),
                        std::move(LHS),
                        std::move(RHS)
                    );
                    break;
                case TokenKind::Mul:
                    LHS = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Mul>>(
                        mergeLocations(LHS->getLocation(), RHS->getLocation()),
                        std::move(LHS),
                        std::move(RHS)
                    );
                    break;
                case TokenKind::Div:
                    LHS = std::make_unique<AstExprBinaryIntToInt<BinaryOpKindIntToInt::Div>>(
                        mergeLocations(LHS->getLocation(), RHS->getLocation()),
                        std::move(LHS),
                        std::move(RHS)
                    );
                    break;
                case TokenKind::Eq:
                    LHS = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Eq>>(
                        mergeLocations(LHS->getLocation(), RHS->getLocation()),
                        std::move(LHS),
                        std::move(RHS)
                    );
                    break;
                case TokenKind::Neq:
                    LHS = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Neq>>(
                        mergeLocations(LHS->getLocation(), RHS->getLocation()),
                        std::move(LHS),
                        std::move(RHS)
                    );
                    break;
                case TokenKind::Leq:
                    LHS = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Leq>>(
                        mergeLocations(LHS->getLocation(), RHS->getLocation()),
                        std::move(LHS),
                        std::move(RHS)
                    );
                    break;
                case TokenKind::Lt:
                    LHS = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Lt>>(
                        mergeLocations(LHS->getLocation(), RHS->getLocation()),
                        std::move(LHS),
                        std::move(RHS)
                    );
                    break;
                case TokenKind::Geq:
                    LHS = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Geq>>(
                        mergeLocations(LHS->getLocation(), RHS->getLocation()),
                        std::move(LHS),
                        std::move(RHS)
                    );
                    break;
                case TokenKind::Gt:
                    LHS = std::make_unique<AstExprBinaryIntToBool<BinaryOpKindIntToBool::Gt>>(
                        mergeLocations(LHS->getLocation(), RHS->getLocation()),
                        std::move(LHS),
                        std::move(RHS)
                    );
                    break;
                case TokenKind::And:
                    LHS = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::And>>(
                        mergeLocations(LHS->getLocation(), RHS->getLocation()),
                        std::move(LHS),
                        std::move(RHS)
                    );
                    break;
                case TokenKind::Or:
                    LHS = std::make_unique<AstExprBinaryBoolToBool<BinaryOpKindBoolToBool::Or>>(
                        mergeLocations(LHS->getLocation(), RHS->getLocation()),
                        std::move(LHS),
                        std::move(RHS)
                    );
                    break;
                default:
                    return std::unique_ptr<AstExpr>(nullptr); // Should not happen
            }
        }
    };
    return parseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<AstExpr> Parser::parsePrimary() {
    Token currentToken = get();
    switch (currentToken.Kind) {
        case TokenKind::Number: {
            auto val = std::make_unique<AstExprConstLong>(currentToken.Location, currentToken.Value);
            nextToken();
            return val;
        }
        case TokenKind::True: {
            auto val = std::make_unique<AstExprConstBool>(currentToken.Location, true);
            nextToken();
            return val;
        }
        case TokenKind::False: {
            auto val = std::make_unique<AstExprConstBool>(currentToken.Location, false);
            nextToken();
            return val;
        }
        case TokenKind::Identifier: {
            return parseCallOrVariable();
        }
        case TokenKind::LParen: {
            nextToken();
            auto expr = parseExpression();
            if (!expr || !consume(TokenKind::RParen)) {
                reportError("Expected ')' after expression");
                return nullptr;
            }
            return expr;
        }
        case TokenKind::Let: return parseLetIn();
        case TokenKind::Match: return parseMatch();
        default:
            reportError("Unknown primary expression");
            return nullptr;
    }
}

std::unique_ptr<AstExpr> Parser::parseLetIn() {
    Token letToken = get();
    nextToken(); // consume 'let'
    if (get().Kind != TokenKind::Identifier) {
        reportError("Expected identifier after 'let'");
        return nullptr;
    }
    std::string varName = get().Text;
    nextToken(); // consume identifier

    if (!consume(TokenKind::Equal)) {
        reportError("Expected '=' after variable name");
        return nullptr;
    }
    auto expr = parseExpression();
    if (!expr) return nullptr;

    if (!consume(TokenKind::In)) {
        reportError("Expected 'in' after expression");
        return nullptr;
    }
    auto body = parseExpression();
    if (!body) return nullptr;

    return std::make_unique<AstExprLetIn>(mergeLocations(letToken.Location, body->getLocation()), varName, std::move(expr), std::move(body));
}

std::unique_ptr<AstExpr> Parser::parseMatch() {
    SourceLocation startLocation = get().Location;
    nextToken(); // consume 'match'
    if (!consume(TokenKind::LBrace)) {
        reportError("Expected '{' after 'match'");
        return nullptr;
    }

    std::vector<std::unique_ptr<AstExprMatchPath>> paths;
    while (get().Kind != TokenKind::RBrace && get().Kind != TokenKind::Eof) {
        auto guard = parseExpression();
        if (!guard) return nullptr;
        if (!consume(TokenKind::Arrow)) {
            reportError("Expected '->' after guard expression");
            return nullptr;
        }
        auto body = parseExpression();
        if (!body) return nullptr;
        paths.push_back(std::make_unique<AstExprMatchPath>(mergeLocations(guard->getLocation(), body->getLocation()), std::move(guard), std::move(body)));
        consume(TokenKind::Comma); // Optional comma
    }

    SourceLocation endLocation = get().Location;
    if (!consume(TokenKind::RBrace)) {
        reportError("Expected '}' after match paths");
        return nullptr;
    }
    return std::make_unique<AstExprMatch>(mergeLocations(startLocation, endLocation), std::move(paths));
}


std::unique_ptr<AstExpr> Parser::parseCallOrVariable() {
    Token nameToken = get();
    nextToken(); // consume identifier

    if (get().Kind != TokenKind::LParen) {
        return std::make_unique<AstExprVariable>(nameToken.Location, nameToken.Text);
    }

    nextToken(); // consume '('
    std::vector<std::unique_ptr<AstExpr>> args;
    if (get().Kind != TokenKind::RParen) {
        while (true) {
            if (auto arg = parseExpression()) {
                args.push_back(std::move(arg));
            } else {
                return nullptr;
            }
            if (get().Kind == TokenKind::RParen) break;
            if (!consume(TokenKind::Comma)) {
                reportError("Expected ',' or ')' in argument list");
                return nullptr;
            }
        }
    }
    Token endToken = get();
    nextToken(); // consume ')'

    return std::make_unique<AstExprCall>(mergeLocations(nameToken.Location, endToken.Location), nameToken.Text, std::move(args));
}

std::unique_ptr<AstPrototype> Parser::parsePrototype() {
    if (get().Kind != TokenKind::Identifier) {
        reportError("Expected function name in prototype");
        return nullptr;
    }
    Token funcNameToken = get();
    nextToken(); // consume name

    if (!consume(TokenKind::LParen)) {
        reportError("Expected '(' after function name");
        return nullptr;
    }

    std::vector<AstArg> args;
    while (get().Kind == TokenKind::Identifier) {
        args.push_back(AstArg {get().Location, get().Text});
        nextToken();
        if (get().Kind == TokenKind::Comma) {
            nextToken();
        }
    }

    Token endToken = get();
    if (!consume(TokenKind::RParen)) {
        reportError("Expected ')' after argument list");
        return nullptr;
    }
    return std::make_unique<AstPrototype>(mergeLocations(funcNameToken.Location, endToken.Location), funcNameToken.Text, std::move(args));
}

std::unique_ptr<AstFunction> Parser::parseFunction() {
    Token startToken = get();
    nextToken(); // consume 'fn'
    auto proto = parsePrototype();
    if (!proto) return nullptr;

    if (!consume(TokenKind::LBrace)) {
        reportError("Expected '{' for function body");
        return nullptr;
    }
    auto body = parseExpression();
    if (!body) return nullptr;

    Token endToken = get();
    if (!consume(TokenKind::RBrace)) {
        reportError("Expected '}' at end of function body");
        return nullptr;
    }

    return std::make_unique<AstFunction>(mergeLocations(startToken.Location, endToken.Location), std::move(proto), std::move(body));
}

std::optional<std::unique_ptr<AstFunction>> Parser::parseTopLevelFunction() {
    if (get().Kind == TokenKind::Fn) {
        return parseFunction();
    }
    reportError("Expected top-level function definition");
    return std::nullopt;
}