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
    size_t startlexerPos = getLexerPosition();
    auto LHS = parsePrimary();
    if (!LHS) throw ParserException("Invalid LHS", SourceLocation {startlexerPos, getLexerPosition()});
    LHS = parsePostfix(std::move(LHS));

    std::function<std::unique_ptr<AstExpr>(int, std::unique_ptr<AstExpr>)> 
    parseBinOpRHS = [&](int exprPrec, std::unique_ptr<AstExpr> LHS) {
        while (true) {
            int tokenPrec = getOpPrecedence(get().Kind);
            if (tokenPrec < exprPrec) return LHS;

            TokenKind binOp = get().Kind;
            nextToken();

            auto RHS = parsePrimary();
            if (!RHS) return std::unique_ptr<AstExpr>(nullptr);
            RHS = parsePostfix(std::move(RHS));

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

std::vector<std::unique_ptr<AstExpr>> Parser::parseCallArgs() {
    std::vector<std::unique_ptr<AstExpr>> args;

    // We assume the '(' has already been consumed by the caller (parsePostfix)
    
    if (get().Kind != TokenKind::RParen) {
        while (true) {
            size_t callArgExprLexerPosition = getLexerPosition();
            if (auto arg = parseExpression()) {
                args.push_back(std::move(arg));
            } else {
                throw ParserException("Invalid Call Argument ", SourceLocation {callArgExprLexerPosition, getLexerPosition()});
            }

            Token nextToken = get();
            if (get().Kind == TokenKind::RParen) break;
            if (!consume(TokenKind::Comma)) {
                throw ParserException("Expected ',' or ')' after argument, found " + tokenToString(nextToken) + " instead", nextToken.Location);
            }
        }
    }
    return args;
}

std::unique_ptr<AstExpr> Parser::parsePostfix(std::unique_ptr<AstExpr> LHS) {
    // Loop to handle chained postfix operators: func(1)[2].field
    while (true) {
        Token currentToken = get();

        // 1. Array Indexing: LHS[index]
        if (currentToken.Kind == TokenKind::LBracket) {
            nextToken(); // Consume '['
            
            auto index = parseExpression();
            if (!index) {
                throw ParserException("Expected index expression in array indexing", get().Location);
            }

            Token endToken = get();
            if (!consume(TokenKind::RBracket)) {
                throw ParserException("Expected ']' to close array index", endToken.Location);
            }
            
            SourceLocation indexLoc = mergeLocations(LHS->getLocation(), endToken.Location);
            LHS = std::make_unique<AstExprIndex>(indexLoc, std::move(LHS), std::move(index));
            // Continue loop for more postfix ops (e.g., array[i][j])
        } 
        
        // 2. Function Call: LHS(args...)
        else if (currentToken.Kind == TokenKind::LParen) {
            nextToken(); // Consume '('
            
            std::vector<std::unique_ptr<AstExpr>> args = parseCallArgs();
            
            Token endToken = get();
            if (!consume(TokenKind::RParen)) {
                throw ParserException("Expected ')' after argument list", endToken.Location);
            }
            
            SourceLocation callLoc = mergeLocations(LHS->getLocation(), endToken.Location);

            if (auto var = dynamic_cast<AstExprVariable*>(LHS.get())) {
                LHS = std::make_unique<AstExprCall>(callLoc, var->getName(), std::move(args));
            } else {
                throw ParserException("Function calls must currently use a identifier as the function.", currentToken.Location);
            }
        }
        // No more postfix operators found
        else {
            return LHS; 
        }
    }
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
            Token nameToken = get();
            nextToken();
            return std::make_unique<AstExprVariable>(nameToken.Location, nameToken.Text);
        }
        case TokenKind::LParen: {
            nextToken();
            auto expr = parseExpression();
            Token end = get();
            if (!expr || !consume(TokenKind::RParen)) {
                throw ParserException("Expected ')' after expression, found " + tokenToString(end) + " instead", end.Location);
            }
            return expr;
        }
        case TokenKind::LBracket: return parseArrayLiteral();
        case TokenKind::Let: return parseLetIn();
        case TokenKind::Match: return parseMatch();
        default:
            return nullptr;
    }
}

std::unique_ptr<AstExpr> Parser::parseLetIn() {
    Token letToken = get();
    nextToken(); // consume 'let'
    if (get().Kind != TokenKind::Identifier) {
        throw ParserException("Expected identifier after 'let', found " + tokenToString(get()) + " instead", get().Location);
    }
    std::string varName = get().Text;
    nextToken(); // consume identifier

    Token equalToken = get();
    if (!consume(TokenKind::Equal)) {
        throw ParserException("Expected '=' after variable name, found " + tokenToString(equalToken) + " instead", equalToken.Location);
    }

    size_t letExprLexerPos = getLexerPosition();
    auto expr = parseExpression();
    if (!expr) throw ParserException("Invalid let expression in let in", SourceLocation {letExprLexerPos, getLexerPosition()});

    Token inToken = get();
    if (!consume(TokenKind::In)) {
        throw ParserException("Expected 'in' after expression, found " + tokenToString(inToken) + " instead", inToken.Location);
    }

    size_t inExprLexerPos = getLexerPosition();
    auto body = parseExpression();
    if (!body) throw ParserException("Invalid in expression in let in", SourceLocation {inExprLexerPos, getLexerPosition()});

    return std::make_unique<AstExprLetIn>(mergeLocations(letToken.Location, body->getLocation()), varName, std::move(expr), std::move(body));
}

std::unique_ptr<AstExpr> Parser::parseMatch() {
    SourceLocation startLocation = get().Location;
    nextToken(); // consume 'match'
    Token lbraceToken = get();
    if (!consume(TokenKind::LBrace)) {
        throw ParserException("Expected '{' after 'match', found " + tokenToString(lbraceToken) + " instead", lbraceToken.Location);
    }

    std::vector<std::unique_ptr<AstExprMatchPath>> paths;
    while (get().Kind != TokenKind::RBrace && get().Kind != TokenKind::Eof) {

        size_t guardExprLexerPos = getLexerPosition();
        auto guard = parseExpression();
        if (!guard) throw ParserException("Invalid guard condition", SourceLocation {guardExprLexerPos, getLexerPosition()});

        Token arrowToken = get();
        if (!consume(TokenKind::Arrow)) {
            throw ParserException("Expected '->' after guard condition, found " + tokenToString(arrowToken) + " instead", arrowToken.Location);
        }

        size_t matchPathExprLexerPos = getLexerPosition();
        auto body = parseExpression();
        if (!body) throw ParserException("Invalid match path expression", SourceLocation {matchPathExprLexerPos, getLexerPosition()});

        paths.push_back(std::make_unique<AstExprMatchPath>(mergeLocations(guard->getLocation(), body->getLocation()), std::move(guard), std::move(body)));
        consume(TokenKind::Comma); // Optional comma
    }

    SourceLocation endLocation = get().Location;
    Token rbraceToken = get();
    if (!consume(TokenKind::RBrace)) {
        throw ParserException("Expected '}' after match paths, found " + tokenToString(rbraceToken) + " instead", rbraceToken.Location);
    }
    return std::make_unique<AstExprMatch>(mergeLocations(startLocation, endLocation), std::move(paths));
}

std::unique_ptr<AstExpr> Parser::parseArrayLiteral() {
    Token startToken = get();
    nextToken(); // Consume '['
    
    std::vector<std::unique_ptr<AstExpr>> elements;
    
    if (get().Kind != TokenKind::RBracket) {
        while (true) {
            auto element = parseExpression();
            if (!element) throw ParserException("Expected array element expression", get().Location);
            elements.push_back(std::move(element));

            if (get().Kind == TokenKind::RBracket) break;
            
            if (!consume(TokenKind::Comma)) {
                throw ParserException("Expected ',' or ']' in array literal", get().Location);
            }
        }
    }
    
    Token endToken = get();
    if (!consume(TokenKind::RBracket)) {
        throw ParserException("Expected ']' at end of array literal", endToken.Location);
    }
    
    SourceLocation arrayLoc = mergeLocations(startToken.Location, endToken.Location);
    auto elementType = std::make_unique<Any>();
    
    return std::make_unique<AstExprConstArray>(arrayLoc, std::move(elementType), std::move(elements));
}

std::unique_ptr<AstPrototype> Parser::parsePrototype() {
    if (get().Kind != TokenKind::Identifier) {
        throw ParserException("Expected function name, found " + tokenToString(get()) + " instead", get().Location);
    }
    Token funcNameToken = get();
    nextToken(); // consume name

    Token lbraceToken = get();
    if (!consume(TokenKind::LParen)) {
        throw ParserException("Expected '(' after function name, found " + tokenToString(lbraceToken) + " instead", lbraceToken.Location);
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
        throw ParserException("Expected ')' after argument list, found " + tokenToString(endToken) + " instead", endToken.Location);
    }
    return std::make_unique<AstPrototype>(mergeLocations(funcNameToken.Location, endToken.Location), funcNameToken.Text, std::move(args));
}

std::unique_ptr<AstFunction> Parser::parseFunction() {
    Token startToken = get();
    nextToken(); // consume 'fn'
    size_t protoStartLexerPos = getLexerPosition();
    auto proto = parsePrototype();
    if (!proto) throw ParserException("Invalid function prototype", SourceLocation {protoStartLexerPos, getLexerPosition()});

    Token lbraceToken = get();
    if (!consume(TokenKind::LBrace)) {
        throw ParserException("Expected '{' after function prototype, found " + tokenToString(lbraceToken) + " instead", lbraceToken.Location);
    }
    size_t bodyStartLexerPos = getLexerPosition();
    auto body = parseExpression();
    if (!body) throw ParserException("Invalid function body", SourceLocation {bodyStartLexerPos, getLexerPosition()});

    Token endToken = get();
    if (!consume(TokenKind::RBrace)) {
        throw ParserException("Expected '}' after function body, found " + tokenToString(endToken) + " instead", endToken.Location);
    }

    return std::make_unique<AstFunction>(mergeLocations(startToken.Location, endToken.Location), std::move(proto), std::move(body));
}

std::optional<std::unique_ptr<AstFunction>> Parser::parseTopLevelFunction() {
    if (get().Kind == TokenKind::Fn) {
        return parseFunction();
    }
    throw ParserException("Expected top function definition, found " + tokenToString(get()) + " instead", get().Location);
}