#ifndef _PARSER_H
#define _PARSER_H

#include <vector>
#include <map>

struct TreeContainer;
class Token;
class IAstExpression;
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

    Token *next(int reset = 0);
    int getTokenPrecedence();

    IAstExpression *parseTopLevelExpression();
    IAstExpression *parseExpression();
    IAstExpression *parseControlFlow();
    IAstExpression *parsePrimary();
    IAstExpression *parseVarExpression();
    IAstExpression *parseParenExpression();
    IAstExpression *parseIdentifierExpression();
    IAstExpression *parseNumberExpression();
    IAstExpression *parseStringExpression();
    IAstExpression *parseBooleanExpression();
    IAstExpression *parseReturnExpression();
    IAstExpression *parseIfElseExpression();
    IAstExpression *parseWhileExpression();
    IAstExpression *parseForExpression();
    IAstExpression *parseBinOpRhs(int precedence, IAstExpression *lhs);
    AstTypeNode *parseTypeNode();
    AstTypeNode *tryInferType(IAstExpression *expr);
    FunctionAst *parseFunctionDefinition();
    PrototypeAst *parsePrototype();
    PrototypeAst *parseExternDeclaration();

    nullptr_t Error(const char *fmt, ...);
    void Warning(const char *fmt, ...);
};

#endif