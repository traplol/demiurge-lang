#ifndef _PARSER_H
#define _PARSER_H

#include <vector>
#include <map>

struct TreeContainer;
class Token;
class IAstExpression;
class AstTypeNode;
class ClassAst;
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
    int _tokenIndex;

    std::map<int, int> _operatorPrecedence;
    std::vector<Token*> _tokens;

    Token *next();
    Token *peek(int offset = 1);
    int getTokenPrecedence();

    IAstExpression *parseTopLevelExpression();
    IAstExpression *parseBlockExpression();
    IAstExpression *parseExpression();
    IAstExpression *parseControlFlow();
    IAstExpression *parsePrimary();
    IAstExpression *parseNewMallocExpression();
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
    IAstExpression *parseArraySubscript();
    IAstExpression *parseBinOpRhs(int precedence, IAstExpression *lhs);
    IAstExpression *parsePrefixUnaryExpr();
    IAstExpression *parsePostfixUnaryExpr(IAstExpression *operand);
    
    FunctionAst *parseFunctionDefinition();
    PrototypeAst *parsePrototype();
    PrototypeAst *parseExternDeclaration();
    ClassAst *parseClassDefinition();
    AstTypeNode *parseTypeNode();
    AstTypeNode *tryInferType(IAstExpression *expr);

    std::nullptr_t Error(const char *fmt, ...);
    void Warning(const char *fmt, ...);
};

#endif
