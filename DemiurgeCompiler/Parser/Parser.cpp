#include <stdarg.h>

#include "../Compiler/TreeContainer.h"
#include "../Lexer/Token.h"
#include "Parser.h"
#include "../Lexer/TokenTypes.h"

#include "../AstNodes/AstBinaryOperatorExpr.h"
#include "../AstNodes/AstBooleanNode.h"
#include "../AstNodes/AstCallExpr.h"
#include "../AstNodes/AstDoubleNode.h"
#include "../AstNodes/AstIfElseExpr.h"
#include "../AstNodes/AstIntegerNode.h"
#include "../AstNodes/AstNodeTypes.h"
#include "../AstNodes/AstReturnExpr.h"
#include "../AstNodes/AstStringNode.h"
#include "../AstNodes/AstTopLevelExpr.h"
#include "../AstNodes/AstTypeNode.h"
#include "../AstNodes/AstVarExpr.h"
#include "../AstNodes/AstVariableNode.h"
#include "../AstNodes/AstWhileExpr.h"
#include "../AstNodes/FunctionAst.h"
#include "../AstNodes/PrototypeAst.h"
#include "../AstNodes/IAstExpression.h"

Parser::Parser() {
    
    // 1 is the lowest operator precedence.
    _operatorPrecedence['='] = 30;              // '='
    _operatorPrecedence[tok_plusequals] = 30;   // '+='
    _operatorPrecedence[tok_minusequals] = 30;  // '-='
    _operatorPrecedence[tok_multequals] = 30;   // '*='
    _operatorPrecedence[tok_divequals] = 30;    // '/='
    _operatorPrecedence[tok_modequals] = 30;    // '%='

    _operatorPrecedence[tok_booleanor] = 40;    // '||'
    _operatorPrecedence[tok_booleanand] = 41;   // '&&'

    _operatorPrecedence['|'] = 45;              // '|'
    _operatorPrecedence['^'] = 46;              // '^'
    _operatorPrecedence['&'] = 47;              // '&'

    _operatorPrecedence[tok_notequal] = 50;     // '!='
    _operatorPrecedence[tok_equalequal] = 50;   // '=='
    _operatorPrecedence['<'] = 60;              // '<'
    _operatorPrecedence[tok_lessequal] = 60;    // '<='
    _operatorPrecedence['>'] = 60;              // '>'
    _operatorPrecedence[tok_greatequal] = 60;   // '>=
    
    _operatorPrecedence[tok_leftshift] = 75;    // '<<'
    _operatorPrecedence[tok_rightshift] = 75;   // '>>'
    _operatorPrecedence['+'] = 80;              // '+'
    _operatorPrecedence['-'] = 80;              // '-'
    _operatorPrecedence['%'] = 100;             // '%'
    _operatorPrecedence['/'] = 100;             // '/'
    _operatorPrecedence['*'] = 100;             // '*'
}


Parser::~Parser() {
}

TreeContainer *Parser::ParseTrees(const std::vector<Token*> &tokens) {
    _tokens = tokens;
    next(1); // setup first token.

    TreeContainer *trees = new TreeContainer();;
    
    while (_curTokenType != EOF) {
        if (_curTokenType == ';') {
            next();
        }
        else if (_curTokenType == tok_func) {
            FunctionAst *func = parseFunctionDefinition();
            if (func == nullptr) {
                return nullptr;
            }
            trees->FunctionDefinitions.push_back(func);
        }
        else if (_curTokenType == tok_extern)  {
            PrototypeAst *declaration = parseExternDeclaration();
            if (declaration == nullptr) {
                return nullptr;
            }
            trees->ExternalDeclarations.push_back(declaration);
        }
        else {
            IAstExpression *toplevel = parseTopLevelExpression();
            if (toplevel == nullptr) {
                return nullptr;
            }
            trees->TopLevelExpressions.push_back(toplevel);
        }
    }
    return trees;
}

Token *Parser::next(int reset) {
    static unsigned i = 0;
    if (reset != 0)
        i = 0;
    if (i >= _tokens.size()) {
        _curToken = nullptr;
        _curTokenType = EOF;
        return nullptr;
    }
    _curToken = _tokens[i++];
    _curTokenType = _curToken->Type();
    return _curToken;
}
int Parser::getTokenPrecedence() {
    if (_operatorPrecedence.count(_curTokenType) == 0) // not an operator, or not in the precedence table.
        return -1;
    return _operatorPrecedence[_curTokenType];
}

nullptr_t Parser::Error(const char *fmt, ...) {
    char buf[4096];
    va_list arg;
    va_start(arg, fmt);
    vsnprintf_s(buf, 4096, fmt, arg);
    va_end(arg);
    fprintf(stderr, "(%d:%d) - Parse Error : Token '%s' : %s\n",
        _curToken->Line(), _curToken->Column(), _curToken->Value().c_str(), buf);
    return nullptr;
}
void Parser::Warning(const char *fmt, ...) {
    char buf[4096];
    va_list arg;
    va_start(arg, fmt);
    vsnprintf_s(buf, 4096, fmt, arg);
    va_end(arg);
    fprintf(stderr, "(%d:%d) - Parse Warning : %s\n",
        _curToken->Line(), _curToken->Column(), buf);
}

IAstExpression *Parser::parseTopLevelExpression() {
    return parseExpression();
}
// <expression>         ::= <primary> <binoprhs>
IAstExpression *Parser::parseExpression() {
    IAstExpression *primary = parsePrimary();
    if (primary != nullptr) {
        IAstExpression *expr = parseBinOpRhs(0, primary);
        if (expr != nullptr) return expr;
    } // if we fail to parse an operation, try to assume it's a control flow statement
    return parseControlFlow();
}

IAstExpression *Parser::parseControlFlow() {
    switch (_curTokenType) {
    default: return Error("Unexpected token.");
    case tok_if: return parseIfElseExpression();
    case tok_while: return parseWhileExpression();
    case tok_for: return parseForExpression();
    case tok_return: return parseReturnExpression();
    case tok_var: return parseVarExpression();
    }
}
IAstExpression *Parser::parsePrimary() {
    switch (_curTokenType) {
    default: return nullptr;/* Error("Unexpected token.");*/
    case '(': return parseParenExpression();
    case ';': next(); return parseExpression();
    case tok_identifier: return parseIdentifierExpression();
    case tok_string: return parseStringExpression();
    case tok_bool: return parseBooleanExpression();
    case tok_number: return parseNumberExpression();
    }
}

// <binoprhs>           ::= ( operator <primary>)*
IAstExpression *Parser::parseBinOpRhs(int precedence, IAstExpression *lhs) {
    while (true) {
        if (_curTokenType == ';') {
            next(); // eat ';'
            return lhs;
        }

        int tokPrec = getTokenPrecedence();
        if (tokPrec < precedence) {
            return lhs;
        }

        std::string operStr = _curToken->Value();
        int binOp = _curTokenType;
        next(); // eat binop
        IAstExpression *rhs = parsePrimary();
        if (rhs == nullptr) return Error("Failed to parse right hand side of expression.");

        int nextPrec = getTokenPrecedence();
        if (tokPrec < nextPrec) {
            rhs = parseBinOpRhs(tokPrec + 1, rhs);
            if (rhs == nullptr) return Error("Failed to parse right hand side of expression.");
        }
        lhs = new AstBinaryOperatorExpr(operStr, (TokenType)binOp, lhs, rhs, _curToken->Line(), _curToken->Column());
    }
}

// <varexpr>            ::= 'var' identifier ':' <type> ';'
//                      |   'var' identifier = <expression> ';'
IAstExpression *Parser::parseVarExpression() {
    if (_curTokenType != tok_var)
        return Error("Expected 'var'.");
    next(); // eat 'var'

    if (_curTokenType != tok_identifier)
        return Error("Expected identifier.");
    std::string identifier = _curToken->Value();
    next(); // eat identifier

    if (_curTokenType == ':') {
        next(); // 'eat ':'
        AstTypeNode *type = parseTypeNode();
        return new AstVarExpr(identifier, nullptr, type, _curToken->Line(), _curToken->Column());
    }
    else if (_curTokenType == '=') {
        next(); // eat '='
        IAstExpression *expression = parseExpression();
        AstTypeNode *type = tryInferType(expression); // if this fails we 
        // will try to infer type at code generation if possible or runtime
        return new AstVarExpr(identifier, expression, type, _curToken->Line(), _curToken->Column());
    }
    else return Error("Expected expression or type declaration when defining a variable.");
}

// <parenexpr>          ::= '(' <expression> ')'
IAstExpression *Parser::parseParenExpression() {
    if (_curTokenType != '(') return Error("Expected '('.");
    next(); // eat '('
    
    IAstExpression *expr = parseExpression();
    if (expr == nullptr) return nullptr;

    if (_curTokenType != ')') return Error("Expected ')'.");
    next(); // eat ')'
    return expr;
}

// <identifier>         ::= identifier
// <callexpr>           ::= identifier '(' <expession> (',' <expression>)* ')'
IAstExpression *Parser::parseIdentifierExpression() {
    if (_curTokenType != tok_identifier)
        return Error("Expected identifier.");
    std::string identifier = _curToken->Value();
    next(); // eat identifier
    if (_curTokenType != '(') // standard identifier reference
        return new AstVariableNode(identifier, _curToken->Line(), _curToken->Column());

    // otherwise it's a call.
    std::vector<IAstExpression*> args;
    next(); // eat '('
    while (_curTokenType != ')') {
        args.push_back(parseExpression());
        if (_curTokenType != ',')
            break;
        next(); // eat ','
    }
    if (_curTokenType != ')')
        return Error("Expected ')' in call.");
    next(); // eat ')'
    return new AstCallExpression(identifier, args, _curToken->Line(), _curToken->Column());
}
// <numberexpr>         ::= number_literal
IAstExpression *Parser::parseNumberExpression() {
    if (_curTokenType != tok_number)
        return Error("Expected number literal.");

    if (_curToken->Value().find('.') != std::string::npos) { // number has a decimal, infer that it is a double type.
        double val = _curToken->AsDouble();
        next(); // eat number
        return new AstDoubleNode(val, _curToken->Line(), _curToken->Column());
    }
    // otherwise assume it's an integer.
    unsigned long long int val = _curToken->AsULong64();
    next(); // eat number
    return new AstIntegerNode(val, _curToken->Line(), _curToken->Column());
}
// <stringexpr>         ::= string_literal
IAstExpression *Parser::parseStringExpression() {
    if (_curTokenType != tok_string)
        return Error("Expected string literal.");
    std::string string = _curToken->Value();
    next(); // eat string
    return new AstStringNode(string, _curToken->Line(), _curToken->Column());
}
// <booleanexpr>        ::= boolean_literal
IAstExpression *Parser::parseBooleanExpression() {
    if (_curTokenType != tok_bool)
        return Error("Expected boolean literal.");
    bool val = _curToken->AsBool();
    next(); // eat bool
    return new AstBooleanNode(val, _curToken->Line(), _curToken->Column());
}
// <returnexpr>         ::= 'return' <expression>
IAstExpression *Parser::parseReturnExpression() {
    if (_curTokenType != tok_return)
        return Error("Expected 'return'.");
    next(); // eat 'return'

    if (_curTokenType == ';') { // void return
        next(); // eat ';'
        return new AstReturnExpr(nullptr, _curToken->Line(), _curToken->Column());
    }

    IAstExpression *expr = parseExpression();
    if (expr == nullptr) return nullptr;

    return new AstReturnExpr(expr, _curToken->Line(), _curToken->Column());
}
// <elseexpr>           ::= 'else' '{' <expression>* '}'
//                      |   'else' <expression>
// <ifexpr>             ::= 'if' <parenexpr> '{' <expression>* '}'
//                      |   'if' <parenexpr> <expression>
IAstExpression *Parser::parseIfElseExpression() {
    if (_curTokenType != tok_if)
        return Error("Expected 'if'.");
    next(); // eat 'if'
    
    IAstExpression *condition = parseParenExpression();

    std::vector<IAstExpression*> ifBody;
    if (_curTokenType != '{') { // single statement
        ifBody.push_back(parseExpression());
    }
    else { // potential multi-statement.
        next(); // eat '{'
        while (_curTokenType != '}') {
            ifBody.push_back(parseExpression());
        }
        next(); // eat '}'
        if (ifBody.size() == 0)
            Warning("Empty 'if' body.");
    }

    std::vector<IAstExpression*> elseBody;
    if (_curTokenType != tok_else) { // assume no else, just fall through.
        return new AstIfElseExpr(condition, ifBody, elseBody, _curToken->Line(), _curToken->Column());
    }
    next(); // eat 'else
    // same as the if body.
    if (_curTokenType != '{') { // single statement
        elseBody.push_back(parseExpression());
    }
    else { // potential multi-statement.
        next(); // eat '{'
        while (_curTokenType != '}') {
            elseBody.push_back(parseExpression());
        }
        next(); // eat '}'
        if (elseBody.size() == 0)
            Warning("Empty 'else' body.");
    }
    return new AstIfElseExpr(condition, ifBody, elseBody, _curToken->Line(), _curToken->Column());
}
// <whileexpr>          ::= 'while' <parenexpr> '{' <expression>* '}'
//                      |   'while' <parenexpr> <expression>
IAstExpression *Parser::parseWhileExpression() {
    if (_curTokenType != tok_while)
        return Error("Expected 'while'.");
    next(); // eat 'while'

    IAstExpression *condition = parseParenExpression();

    std::vector<IAstExpression*> whileBody;
    if (_curTokenType != '{') { // single statement
        whileBody.push_back(parseExpression());
    }
    else { // potential multi-statement.
        next(); // eat '{'
        while (_curTokenType != '}') {
            whileBody.push_back(parseExpression());
        }
        next(); // eat '}'
        if (whileBody.size() == 0)
            Warning("Empty 'while' body.");
    }

    return new AstWhileExpr(condition, whileBody, _curToken->Line(), _curToken->Column());
}
// <forexpr>            ::=
IAstExpression *Parser::parseForExpression() {
    return Error("'for' expression not yet implemented.");
}
// <type>               ::= 'double' | 'int' | 'bool' | 'string'
AstTypeNode *Parser::parseTypeNode() {
    int curTokType = _curTokenType;
    switch (curTokType) {
    default: return nullptr;
    case tok_typevoid: next(); return new AstTypeNode(node_void, _curToken->Line(), _curToken->Column());
    case tok_typebool: next(); return new AstTypeNode(node_boolean, _curToken->Line(), _curToken->Column());
    case tok_typedouble: next(); return new AstTypeNode(node_double, _curToken->Line(), _curToken->Column());
    case tok_typeint: next(); return new AstTypeNode(node_integer, _curToken->Line(), _curToken->Column());
    case tok_typestring: next(); return new AstTypeNode(node_string, _curToken->Line(), _curToken->Column());
    }
}
// Attempts to generate an AstTypeNode from the expression's type.
AstTypeNode *Parser::tryInferType(IAstExpression *expr) {
    switch (expr->getNodeType()) {
    default: return nullptr;
    case node_void: return new AstTypeNode(node_void, _curToken->Line(), _curToken->Column());
    case node_boolean: return new AstTypeNode(node_boolean, _curToken->Line(), _curToken->Column());
    case node_double: return new AstTypeNode(node_double, _curToken->Line(), _curToken->Column());
    case node_integer: return new AstTypeNode(node_integer, _curToken->Line(), _curToken->Column());
    case node_string: return new AstTypeNode(node_string, _curToken->Line(), _curToken->Column());
    }
}

// <functionast>        ::= <prototype>  '{' <expression>* '}'
FunctionAst *Parser::parseFunctionDefinition() {
    PrototypeAst *proto = parsePrototype();
    if (proto == nullptr) return nullptr;

    if (_curTokenType != '{')
        return Error("Expected '{' at start of function body.");
    next(); // eat '{'

    std::vector<IAstExpression*> functionBody;
    while (_curTokenType != '}') {
        functionBody.push_back(parseExpression());
    }
    next(); // eat '}'

    if (functionBody.size() == 0)
        Warning("Empty function body.");

    return new FunctionAst(proto, functionBody, _curToken->Line(), _curToken->Column());
}

// let id := identifier
// <prototype>          ::= 'func' id '(' id ':' <type> (',' id ':' <type>)* ')' ':' <type>
PrototypeAst *Parser::parsePrototype() {
    if (_curTokenType != tok_func)
        return Error("Expected 'func'.");
    next(); // eat 'func'

    if (_curTokenType != tok_identifier)
        return Error("Expected identifier in function prototype.");
    std::string functionIdentifier = _curToken->Value();
    next(); // eat identifier

    if (_curTokenType != '(')
        return Error("Expected '('.");
    next(); // eat '('
    std::vector<std::pair<std::string, AstTypeNode*>> args;
    while (_curTokenType != ')') { // this while loop short circuits zero parameter functions
        if (_curTokenType != tok_identifier)
            return Error("Expected identifier in parameter list.");
        std::string argName = _curToken->Value();
        next(); // eat identifier
        if (_curTokenType != ':')
            return Error("Expected ':' for parameter type definition.");
        next(); // eat ':'
        AstTypeNode *argType = parseTypeNode();

        auto pair = std::make_pair(argName, argType);
        args.push_back(pair);
        if (_curTokenType != ',')
            break;
        next(); // eat ','
    }

    if (_curTokenType != ')')
        return Error("Expected ')'.");
    next(); // eat ')'

    if (_curTokenType != ':')
        return Error("Expected ':' for function return type.");
    next(); // eat ':'

    AstTypeNode *returnType = parseTypeNode();
    if (returnType == nullptr)
        return Error("Expected function return type.");

    return new PrototypeAst(functionIdentifier, returnType, args, false, _curToken->Line(), _curToken->Column());
}

// <extern>          ::= 'extern' 'func' id '(' (id ':')? <type> (',' (id ':')? <type>)* (',' '...')? ')' ':' <type>
PrototypeAst *Parser::parseExternDeclaration() {
    if (_curTokenType != tok_extern)
        return Error("Expected 'extern'.");
    next(); // eat 'extern'

    if (_curTokenType != tok_func)
        return Error("Expected 'func'.");
    next(); // eat 'func'

    if (_curTokenType != tok_identifier)
        return Error("Expected function identifier in external declaration.");
    std::string functionIdentifier = _curToken->Value();
    next(); // eat identifier

    if (_curTokenType != '(')
        return Error("Expected '('.");
    next(); // eat '('
    bool isVarArgs = false;
    std::vector<std::pair<std::string, AstTypeNode*>> args;
    int i = 0;
    while (_curTokenType != ')') { // this while loop short circuits zero parameter functions
        if (isVarArgs)
            return Error("Varags, '...', must be at the end of parameter list.");
        if (_curTokenType == tok_dotdotdot) { // found a varargs
            isVarArgs = true;
            next(); // eat '...'
        }
        else { // should be a type.
            if (_curTokenType == tok_identifier) // identifiers are optional in externs.
            {
                next(); // eat identifier
                if (_curTokenType != ':')
                    return Error("Expected ':' after parameter name.");
                next(); // eat ':'
            }
            AstTypeNode *argType = parseTypeNode();
            if (argType == nullptr)
                return Error("Expected type identitifer in external declaration.");
            auto pair = std::make_pair("param"+i, argType);
            args.push_back(pair);
        }
        if (_curTokenType != ',')
            break;
        next(); // eat ','
        i++;
    }

    if (_curTokenType != ')')
        return Error("Expected ')'.");
    next(); // eat ')'

    if (_curTokenType != ':')
        return Error("Expected ':' for function return type.");
    next(); // eat ':'

    AstTypeNode *returnType = parseTypeNode();
    if (returnType == nullptr)
        return Error("Expected function return type.");

    if (_curTokenType != ';')
        return Error("Expected ';'");
    next(); // eat ';'

    return new PrototypeAst(functionIdentifier, returnType, args, isVarArgs, _curToken->Line(), _curToken->Column());
}