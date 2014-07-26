#ifndef _PARSER_H
#define _PARSER_H

#include <vector>
#include <map>

struct TreeContainer;
class Token;
class IExpressionAST;
class AstTypeNode;
class FunctionAst;
class PrototypeAst;

class Parser {
public:
    Parser();
    ~Parser();
    TreeContainer *ParseTrees(const std::vector<Token*> &tokens);

private:
    Token *_curToken;
    int _curTokenType;

    std::map<int, int> _operatorPrecedence;
    std::vector<Token*> _tokens;

    Token *next();
    int getTokenPrecedence();

    IExpressionAST *parseTopLevelExpression();
    IExpressionAST *parseExpression();
    IExpressionAST *parseControlFlow();
    IExpressionAST *parsePrimary();
    IExpressionAST *parseVarExpression();
    IExpressionAST *parseParenExpression();
    IExpressionAST *parseIdentifierExpression();
    IExpressionAST *parseNumberExpression();
    IExpressionAST *parseStringExpression();
    IExpressionAST *parseBooleanExpression();
    IExpressionAST *parseReturnExpression();
    IExpressionAST *parseIfElseExpression();
    IExpressionAST *parseWhileExpression();
    IExpressionAST *parseForExpression();
    IExpressionAST *parseBinOpRhs(int precedence, IExpressionAST *lhs);
    AstTypeNode *parseTypeNode();
    AstTypeNode *tryInferType(IExpressionAST *expr);
    FunctionAst *parseFunctionDefinition();
    PrototypeAst *parsePrototype();

    nullptr_t Error(const char *fmt, ...);
    void Warning(const char *fmt, ...);
};

#endif