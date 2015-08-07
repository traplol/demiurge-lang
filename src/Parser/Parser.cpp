#include <stdarg.h>
#include "Parser/Parser.h"

#include "Compiler/TreeContainer.h"
#include "Lexer/Token.h"
#include "Lexer/TokenTypes.h"

#include "AstNodes/AstBinaryOperatorExpr.h"
#include "AstNodes/AstUnaryOperatorExpr.h"
#include "AstNodes/AstBooleanNode.h"
#include "AstNodes/AstCallExpr.h"
#include "AstNodes/AstDoubleNode.h"
#include "AstNodes/AstIfElseExpr.h"
#include "AstNodes/AstIntegerNode.h"
#include "AstNodes/AstNodeTypes.h"
#include "AstNodes/AstReturnExpr.h"
#include "AstNodes/AstStringNode.h"
#include "AstNodes/AstTopLevelExpr.h"
#include "AstNodes/AstTypeNode.h"
#include "AstNodes/AstVarExpr.h"
#include "AstNodes/AstVariableNode.h"
#include "AstNodes/AstWhileExpr.h"
#include "AstNodes/AstForExpr.h"
#include "AstNodes/FunctionAst.h"
#include "AstNodes/PrototypeAst.h"
#include "AstNodes/ClassAst.h"
#include "AstNodes/IAstExpression.h"

Parser::Parser() {
    
    // 1 is the lowest operator precedence.
    _operatorPrecedence['='] = 30;                  // '='
    _operatorPrecedence[tok_leftshiftequal] = 30;   // '<<='
    _operatorPrecedence[tok_rightshiftequal] = 30;  // '>>='
    _operatorPrecedence[tok_andequals] = 30;        // '&='
    _operatorPrecedence[tok_orequals] = 30;         // '|='
    _operatorPrecedence[tok_xorequals] = 30;        // '^='
    _operatorPrecedence[tok_plusequals] = 30;       // '+='
    _operatorPrecedence[tok_minusequals] = 30;      // '-='
    _operatorPrecedence[tok_multequals] = 30;       // '*='
    _operatorPrecedence[tok_divequals] = 30;        // '/='
    _operatorPrecedence[tok_modequals] = 30;        // '%='

    _operatorPrecedence[tok_booleanor] = 40;        // '||'
    _operatorPrecedence[tok_booleanand] = 41;       // '&&'

    _operatorPrecedence['|'] = 45;                  // '|'
    _operatorPrecedence['^'] = 46;                  // '^'
    _operatorPrecedence['&'] = 47;                  // '&'

    _operatorPrecedence[tok_notequal] = 50;         // '!='
    _operatorPrecedence[tok_equalequal] = 50;       // '=='
    _operatorPrecedence['<'] = 60;                  // '<'
    _operatorPrecedence[tok_lessequal] = 60;        // '<='
    _operatorPrecedence['>'] = 60;                  // '>'
    _operatorPrecedence[tok_greatequal] = 60;       // '>=
    
    _operatorPrecedence[tok_leftshift] = 75;        // '<<'
    _operatorPrecedence[tok_rightshift] = 75;       // '>>'
    _operatorPrecedence['+'] = 80;                  // '+'
    _operatorPrecedence['-'] = 80;                  // '-'
    _operatorPrecedence['%'] = 100;                 // '%'
    _operatorPrecedence['/'] = 100;                 // '/'
    _operatorPrecedence['*'] = 100;                 // '*'
    
    _operatorPrecedence['['] = 120;                 // '['
    _operatorPrecedence['.'] = 120;                 // '.'
}


Parser::~Parser() {
}

TreeContainer *Parser::ParseTrees(const std::vector<Token*> &tokens) {
    _tokenIndex = 0;
    _tokens = tokens;
    next(); // setup first token.

    TreeContainer *trees = new TreeContainer();;
    
    while (_curTokenType != EOF) {
        if (_curTokenType == ';') {
            next();
        }
        else if (_curTokenType == tok_class) {
            ClassAst *class_ = parseClassDefinition();
            if (class_ == nullptr) {
                delete trees;
                return nullptr;
            }
            trees->ClassDefinitions.push_back(class_);
        }
        else if (_curTokenType == tok_func) {
            FunctionAst *func = parseFunctionDefinition();
            if (func == nullptr) {
                delete trees;
                return nullptr;
            }
            trees->FunctionDefinitions.push_back(func);
        }
        else if (_curTokenType == tok_extern)  {
            PrototypeAst *declaration = parseExternDeclaration();
            if (declaration == nullptr) {
                delete trees;
                return nullptr;
            }
            trees->ExternalDeclarations.push_back(declaration);
        }
        else {
            IAstExpression *toplevel = parseTopLevelExpression();
            if (toplevel == nullptr) {
                delete trees;
                return nullptr;
            }
            trees->TopLevelExpressions.push_back(toplevel);
        }
    }
    return trees;
}

Token *Parser::next() {
    if (_tokenIndex >= _tokens.size()) {
        _curToken = nullptr;
        _curTokenType = EOF;
        return nullptr;
    }
    _curToken = _tokens[_tokenIndex++];
    _curTokenType = _curToken->Type();
    return _curToken;
}
Token *Parser::peek(int offset) {
    if (_tokenIndex + offset >= _tokens.size()) {
        return nullptr;
    }
    return _tokens[_tokenIndex + offset];
}
int Parser::getTokenPrecedence() {
    if (_operatorPrecedence.count(_curTokenType) == 0) // not an operator, or not in the precedence table.
        return -1;
    return _operatorPrecedence[_curTokenType];
}

std::nullptr_t Parser::Error(const char *fmt, ...) {
    char buf[4096];
    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buf, 4096, fmt, arg);
    va_end(arg);
    fprintf(stderr, "(%d:%d) - Parse Error : Token '%s' : %s\n",
        _curToken->Line(), _curToken->Column(), _curToken->Value().c_str(), buf);
    return nullptr;
}
void Parser::Warning(const char *fmt, ...) {
    char buf[4096];
    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buf, 4096, fmt, arg);
    va_end(arg);
    fprintf(stderr, "(%d:%d) - Parse Warning : %s\n",
        _curToken->Line(), _curToken->Column(), buf);
}

IAstExpression *Parser::parseTopLevelExpression() {
    return parseExpression();
}

// <blockexpr>          ::= <expression>
//                      |   <controlflow>
IAstExpression *Parser::parseBlockExpression() {
    IAstExpression *retExpr = parseExpression();
    if (retExpr == nullptr) {
        retExpr = parseControlFlow();
    }

    if (_curTokenType == ';') {
        next(); // eat optional ';'
    }
    return retExpr;
}

// <expression>         ::= <primary> <binoprhs>
//                      |   <unaryop> <primary>
//                      |   <primary> <unaryop>
IAstExpression *Parser::parseExpression() {
    IAstExpression *unaryPre = parsePrefixUnaryExpr();
    if (unaryPre != nullptr) { // if we parsed a unary expression, see if it's used in a binary operation
        return parseBinOpRhs(0, unaryPre);
    }

    IAstExpression *primary = parsePrimary();
    if (primary != nullptr) {
        IAstExpression *unaryPost = parsePostfixUnaryExpr(primary); 
        if (unaryPost != nullptr) { // if we parsed a unary expression, see if it's used in a binary operation
            return parseBinOpRhs(0, unaryPost);
        }
        return parseBinOpRhs(0, primary); // otherwise just keep parsing the binop
    }
    return nullptr;
}

// <controlflow>        ::= <ifelseexpr>
//                      |   <whileexpr>
//                      |   <forexpr>
//                      |   <varexpr>
//                      |   <returnexpr>
IAstExpression *Parser::parseControlFlow() {
    switch (_curTokenType) {
    default: return nullptr;
    case ';': next(); return parseExpression();
    case tok_if: return parseIfElseExpression();
    case tok_while: return parseWhileExpression();
    case tok_for: return parseForExpression();
    case tok_var: return parseVarExpression();
    case tok_return: return parseReturnExpression();
    }
}

// <primary>            ::= <parenexpr>
//                      |   <identifier>
//                      |   <string>
//                      |   <bool>
//                      |   <number>
IAstExpression *Parser::parsePrimary() {
    if (_curToken->IsUnaryOperator())
    {
        IAstExpression *unary = parsePrefixUnaryExpr();
        if (unary != nullptr) {
            return unary;
        }
    }
    switch (_curTokenType) {
    default: return nullptr;
    case '(': return parseParenExpression();
    case ';': next(); return parseExpression();
    case tok_identifier: return parseIdentifierExpression();
    case tok_string: return parseStringExpression();
    case tok_bool: return parseBooleanExpression();
    case tok_number: return parseNumberExpression();
    }
}

// <prefixunary>        ::= unaryoper <primary>
IAstExpression *Parser::parsePrefixUnaryExpr() {
    if (!_curToken->IsUnaryOperator()) { // short curcuit if not an unary operator.
        return nullptr;
    }

    std::string oper = _curToken->Value();
    int tokenType = _curTokenType;
    IAstExpression *operand;
    if (tokenType == tok_new) {
        return parseNewMallocExpression();
    }
    else {
        next(); // eat unaryoper
        operand = parsePrimary();
        if (operand == nullptr) {
            return nullptr;
        }
    }
    return new AstUnaryOperatorExpr(oper, (TokenType)tokenType, operand, false, _curToken->Line(), _curToken->Column());
}

// <prefixunary>        ::= <primary> unaryoper 
IAstExpression *Parser::parsePostfixUnaryExpr(IAstExpression *operand) {
    if (!_curToken->IsUnaryOperator() || operand == nullptr) {// short curcuit if not an operator.
        return nullptr;
    }

    // the only 'postfix' operators should be '++', '--' and '[<expression>]'
    if (_curTokenType != tok_plusplus && _curTokenType != tok_minusminus && _curTokenType != '[') {
        return nullptr;
    }

    std::string oper = _curToken->Value();
    int tokenType = _curTokenType; 
    if (_curTokenType == tok_plusplus || _curTokenType == tok_minusminus) { // '++' or '--'
        next(); // eat unaryoper
        return new AstUnaryOperatorExpr(oper, (TokenType)tokenType, operand, true, _curToken->Line(), _curToken->Column());
    }
    // otherwise should be an index operator, '[' <expression> ']'
    IAstExpression *indexExpr = parseArraySubscript();
    return new AstUnaryOperatorExpr(oper, (TokenType)tokenType, operand, true, indexExpr, _curToken->Line(), _curToken->Column());
}

// <binoprhs>           ::= ( operator <expression>)*
IAstExpression *Parser::parseBinOpRhs(int precedence, IAstExpression *lhs) {
    while (true) {
        if (_curTokenType == ';') {
            return lhs;
        }

        if (_curTokenType == '[') {
            lhs = parsePostfixUnaryExpr(lhs);
        }

        int tokPrec = getTokenPrecedence();
        if (tokPrec < precedence) {
            return lhs;
        }

        std::string operStr = _curToken->Value();
        int binOp = _curTokenType;
        next(); // eat binop
        IAstExpression *rhs = parsePrimary();
        if (rhs == nullptr) {
            return Error("Failed to parse right hand side of expression.");
        }

        int nextPrec = getTokenPrecedence();
        if (tokPrec < nextPrec) {
            rhs = parseBinOpRhs(tokPrec + 1, rhs);
            if (rhs == nullptr) {
                return Error("Failed to parse right hand side of expression.");
            }
        }
        lhs = new AstBinaryOperatorExpr(operStr, (TokenType)binOp, lhs, rhs, _curToken->Line(), _curToken->Column());
    }
}

// <varexpr>            ::= 'var' identifier ':' <type> ';'
//                      |   'var' identifier = <expression> ';'
IAstExpression *Parser::parseVarExpression() {
    if (_curTokenType != tok_var) {
        return Error("Expected 'var'.");
    }
    next(); // eat 'var'

    if (_curTokenType != tok_identifier) {
        return Error("Expected identifier.");
    }
    std::string identifier = _curToken->Value();
    next(); // eat identifier

    if (_curTokenType == ':') { // "var x : int"
        next(); // 'eat ':'
        AstTypeNode *type = parseTypeNode();
        if (type == nullptr) {
            return Error("Could not parse variable type.");
        }
        return new AstVarExpr(identifier, nullptr, type, _curToken->Line(), _curToken->Column());
    }
    else if (_curTokenType == '=') { // "var x = 5"
        next(); // eat '='
        IAstExpression *expression = parseExpression();
        // if this fails we will try to infer type at code generation if possible or runtime
        AstTypeNode *type = tryInferType(expression);
        return new AstVarExpr(identifier, expression, type, _curToken->Line(), _curToken->Column());
    }
    else return Error("Expected assignment expression or type declaration when defining a variable.");
}

// <parenexpr>          ::= '(' <expression> ')'
IAstExpression *Parser::parseParenExpression() {
    if (_curTokenType != '(') {
        return Error("Expected '('.");
    }
    next(); // eat '('
    
    IAstExpression *expr = parseExpression();
    if (expr == nullptr) {
        return nullptr;
    }

    if (_curTokenType != ')') {
        return Error("Expected ')'.");
    }
    next(); // eat ')'
    return expr;
}

// <identifier>         ::= identifier
// <callexpr>           ::= identifier '(' <expession> (',' <expression>)* ')'
IAstExpression *Parser::parseIdentifierExpression() {
    if (_curTokenType != tok_identifier) {
        return Error("Expected identifier.");
    }
    std::string identifier = _curToken->Value();
    next(); // eat identifier
    if (_curTokenType != '(') { // standard identifier reference
        return new AstVariableNode(identifier, _curToken->Line(), _curToken->Column());
    }

    // otherwise it's a call.
    std::vector<IAstExpression*> args;
    next(); // eat '('
    while (_curTokenType != ')') {
        args.push_back(parseExpression());
        if (_curTokenType != ',') {
            break;
        }
        next(); // eat ','
    }
    if (_curTokenType != ')') {
        return Error("Expected ')' in call.");
    }
    next(); // eat ')'
    return new AstCallExpression(identifier, args, _curToken->Line(), _curToken->Column());
}

// <numberexpr>         ::= number_literal
IAstExpression *Parser::parseNumberExpression() {
    if (_curTokenType != tok_number) {
        return Error("Expected number literal.");
    }

    if (_curToken->Value().find('.') != std::string::npos) { // number has a decimal, infer that it is a double type.
        double val = _curToken->AsDouble();
        next(); // eat number
        return new AstDoubleNode(val, _curToken->Line(), _curToken->Column());
    }
    // otherwise assume it's an integer.
    demi_int val = _curToken->AsULong64();
    next(); // eat number
    return new AstIntegerNode(val, _curToken->Line(), _curToken->Column());
}

// <stringexpr>         ::= string_literal
IAstExpression *Parser::parseStringExpression() {
    if (_curTokenType != tok_string) {
        return Error("Expected string literal.");
    }
    std::string string = _curToken->Value();
    next(); // eat string
    return new AstStringNode(string, _curToken->Line(), _curToken->Column());
}

// <booleanexpr>        ::= boolean_literal
IAstExpression *Parser::parseBooleanExpression() {
    if (_curTokenType != tok_bool) {
        return Error("Expected boolean literal.");
    }
    bool val = _curToken->AsBool();
    next(); // eat bool
    return new AstBooleanNode(val, _curToken->Line(), _curToken->Column());
}

// <returnexpr>         ::= 'return' <expression>
IAstExpression *Parser::parseReturnExpression() {
    if (_curTokenType != tok_return) {
        return Error("Expected 'return'.");
    }
    next(); // eat 'return'
    IAstExpression *expr = parseExpression(); // void return will automatically be handled.
    return new AstReturnExpr(expr, _curToken->Line(), _curToken->Column());
}

// <elseexpr>           ::= 'else' '{' <expression>* '}'
//                      |   'else' <expression>
// <ifexpr>             ::= 'if' <parenexpr> '{' <expression>* '}'
//                      |   'if' <parenexpr> <expression>
IAstExpression *Parser::parseIfElseExpression() {
    if (_curTokenType != tok_if) {
        return Error("Expected 'if'.");
    }
    next(); // eat 'if'
    
    IAstExpression *condition = parseParenExpression();

    std::vector<IAstExpression*> ifBody;
    if (_curTokenType != '{') { // single statement
        ifBody.push_back(parseBlockExpression());
    }
    else { // potential multi-statement.
        next(); // eat '{'
        while (_curTokenType != '}') {
            ifBody.push_back(parseBlockExpression());
        }
        next(); // eat '}'
        if (ifBody.size() == 0) {
            Warning("Empty 'if' body.");
        }
    }

    std::vector<IAstExpression*> elseBody;
    if (_curTokenType != tok_else) { // assume no else, just fall through.
        return new AstIfElseExpr(condition, ifBody, elseBody, _curToken->Line(), _curToken->Column());
    }
    next(); // eat 'else
    // same as the if body.
    if (_curTokenType != '{') { // single statement
        elseBody.push_back(parseBlockExpression());
    }
    else { // potential multi-statement.
        next(); // eat '{'
        while (_curTokenType != '}') {
            elseBody.push_back(parseBlockExpression());
        }
        next(); // eat '}'
        if (elseBody.size() == 0) {
            Warning("Empty 'else' body.");
        }
    }
    return new AstIfElseExpr(condition, ifBody, elseBody, _curToken->Line(), _curToken->Column());
}

// <whileexpr>          ::= 'while' <parenexpr> '{' <expression>* '}'
//                      |   'while' <parenexpr> <expression>
IAstExpression *Parser::parseWhileExpression() {
    if (_curTokenType != tok_while) {
        return Error("Expected 'while'.");
    }
    next(); // eat 'while'

    IAstExpression *condition = parseParenExpression();

    std::vector<IAstExpression*> whileBody;
    if (_curTokenType != '{') { // single statement
        whileBody.push_back(parseBlockExpression());
    }
    else { // potential multi-statement.
        next(); // eat '{'
        while (_curTokenType != '}') {
            whileBody.push_back(parseBlockExpression());
        }
        next(); // eat '}'
        if (whileBody.size() == 0) {
            Warning("Empty 'while' body.");
        }
    }

    return new AstWhileExpr(condition, whileBody, _curToken->Line(), _curToken->Column());
}

// <forexpr>            ::= 'for' '(' 
//                          <expression> (',' <expression>)* ';'    -- Init
//                          <expression> ';'                        -- Condition
//                          <expression> (',' <expression>)*        -- Afterthought
//                          ')'         
//                          '{' <expression>* '}'                   -- Body
IAstExpression *Parser::parseForExpression() {
    if (_curTokenType != tok_for) {
        return Error("Expected 'for'");
    }
    next(); // eat 'for'

    if (_curTokenType != '(') {
        return Error("Expected '('");
    }
    next(); // eat '('

    std::vector<IAstExpression*> init;
    IAstExpression* condition;
    std::vector<IAstExpression*> afterthough;
    
    // Parse the for loop initialization.
    while (_curTokenType != ';') {
        IAstExpression *expr = parseExpression();
        if (expr == nullptr) {
            expr = parseVarExpression();
        }
        init.push_back(expr);
        if (_curTokenType != ',') {
            break;
        }
        next(); // eat ','
    }
    if (_curTokenType != ';') {
        return Error("Expected ';'.");
    }
    next(); // eat ';'

    // Parse the for loop condition.
    condition = parseExpression();
    if (condition == nullptr) {
        return Error("Failed to parse 'for' condition.");
    }
    if (_curTokenType != ';') {
        return Error("Expected ';'.");
    }
    next(); // eat ';'
    // Parse the for loop afterthought
    while (_curTokenType != ')') {
        afterthough.push_back(parseExpression());
        if (_curTokenType != ',') {
            break;
        }
        next(); // eat ','
    }
    if (_curTokenType != ')') {
        return Error("Expected ')'.");
    }
    next(); // eat ')'

    // Parse the for loop body.
    std::vector<IAstExpression*> body;
    if (_curTokenType != '{') { // Assume single statement for loop.
        body.push_back(parseExpression());
    }
    else { // has '{'
        next(); // eat '{'
        while (_curTokenType != '}') {
            body.push_back(parseBlockExpression());
        }
        next(); // eat '}'
    }
    return new AstForExpr(init, condition, afterthough, body, _curToken->Line(), _curToken->Column());
}

// <arraysubscript>     ::= '[' <expression> ']'
IAstExpression *Parser::parseArraySubscript() {
    if (_curTokenType != '[') {
        return Error("Expected '['.");
    }
    next(); // eat '['
    IAstExpression *expr = parseExpression();
    next(); // eat ']'
    return expr;
}

IAstExpression *Parser::parseNewMallocExpression() {
    if (_curTokenType != tok_new) {
        return Error("Expected 'new'.");
    }
    next(); // eat 'new'
    AstTypeNode *newType = parseTypeNode();
    if (newType == nullptr) {
        return Error("Expected type.");
    }
    return new AstUnaryOperatorExpr("new", tok_new, newType, false, _curToken->Line(), _curToken->Column());
}

// <type>               ::= ( identifier | <reserved type> ) ( '[' <numberexpr>? ']' )?
AstTypeNode *Parser::parseTypeNode() {
    int tokType = _curTokenType;
    std::string typeName = _curToken->Value();
    next(); // eat type

    AstNodeType nodeType;
    IAstExpression *subscript = nullptr;
    demi_int arraySize = 0;
    bool isArray = _curTokenType == '[' || _curTokenType == tok_LRSqBrackets;
    
    if (_curTokenType == tok_LRSqBrackets) { 
        next(); // eat '[]'
    } 
    else if (_curTokenType == '[') {
        subscript = parseArraySubscript();
        AstIntegerNode *size = dynamic_cast<AstIntegerNode*>(subscript);
        if (size != nullptr) { // the subscript is a number, go ahead and assume this is a static array.
            arraySize = size->getValue();
            if (arraySize <= 0) {
                return Error("Static array size must be greater than zero.");
            }
        }
        else { // Otherwise 

        }
    }
    switch (tokType) {
    default: return nullptr;
    case tok_identifier: nodeType = node_struct; break;
    case tok_typevoid: nodeType = node_void; break;
    
    case tok_typefloat: nodeType = node_float; break;
    case tok_typedouble: nodeType = node_double; break;

    case tok_typebool: nodeType = node_boolean; break;
    case tok_typeint8: nodeType = node_signed_integer8; break;
    case tok_typeint16: nodeType = node_signed_integer16; break;
    case tok_typeint32: nodeType = node_signed_integer32; break;
    case tok_typeint64: nodeType = node_signed_integer64; break;

    case tok_typeuint8: nodeType = node_unsigned_integer8; break;
    case tok_typeuint16: nodeType = node_unsigned_integer16; break;
    case tok_typeuint32: nodeType = node_unsigned_integer32; break;
    case tok_typeuint64: nodeType = node_unsigned_integer64; break;

    case tok_typestring: nodeType = node_string; break;
    }
    if (isArray && arraySize > 0) { // static sized array.
        return new AstTypeNode(nodeType, typeName, isArray, arraySize, _curToken->Line(), _curToken->Column());
    }
    if (isArray && arraySize == 0) { // 'new' array.
        return new AstTypeNode(nodeType, typeName, isArray, subscript, _curToken->Line(), _curToken->Column());
    }
    // Some other type.
    return new AstTypeNode(nodeType, typeName, _curToken->Line(), _curToken->Column());
}

// Attempts to generate an AstTypeNode from the expression's type.
AstTypeNode *Parser::tryInferType(IAstExpression *expr) {
    switch (expr->getNodeType()) {
    default: return nullptr;
    case node_void: return new AstTypeNode(node_void, "void", _curToken->Line(), _curToken->Column());
    case node_boolean: return new AstTypeNode(node_boolean, "bool", _curToken->Line(), _curToken->Column());
    case node_double: return new AstTypeNode(node_double, "double", _curToken->Line(), _curToken->Column());
    case node_float: return new AstTypeNode(node_float, "float", _curToken->Line(), _curToken->Column());
    
    case node_signed_integer8: return new AstTypeNode(node_signed_integer8, "int8", _curToken->Line(), _curToken->Column());
    case node_signed_integer16: return new AstTypeNode(node_signed_integer16, "int16", _curToken->Line(), _curToken->Column());
    case node_signed_integer32: return new AstTypeNode(node_signed_integer32, "int32", _curToken->Line(), _curToken->Column());
    case node_signed_integer64: return new AstTypeNode(node_signed_integer64, "int64", _curToken->Line(), _curToken->Column());
              
    case node_unsigned_integer8: return new AstTypeNode(node_unsigned_integer8, "uint8", _curToken->Line(), _curToken->Column());
    case node_unsigned_integer16: return new AstTypeNode(node_unsigned_integer16, "uint16", _curToken->Line(), _curToken->Column());
    case node_unsigned_integer32: return new AstTypeNode(node_unsigned_integer32, "uint32", _curToken->Line(), _curToken->Column());
    case node_unsigned_integer64: return new AstTypeNode(node_unsigned_integer64, "uint64", _curToken->Line(), _curToken->Column());

    case node_string: return new AstTypeNode(node_string, "string", _curToken->Line(), _curToken->Column());
    }
}

// <functionast>        ::= <prototype>  '{' <expression>* '}'
FunctionAst *Parser::parseFunctionDefinition() {
    PrototypeAst *proto = parsePrototype();
    if (proto == nullptr) {
        return nullptr;
    }
    if (_curTokenType != '{') {
        return Error("Expected '{' at start of function body.");
    }
    next(); // eat '{'

    std::vector<IAstExpression*> functionBody;
    while (_curTokenType != '}') {
        IAstExpression *blockExpr = parseBlockExpression();
        if (blockExpr == nullptr) {
            return Error("Unexpected token.");
        }
        functionBody.push_back(blockExpr);
    }
    next(); // eat '}'

    if (functionBody.size() == 0) {
        Warning("Empty function body.");
    }

    return new FunctionAst(proto, functionBody, _curToken->Line(), _curToken->Column());
}

// let id := identifier
// <prototype>          ::= 'func' id '(' id ':' <type> (',' id ':' <type>)* ')' ':' <type>
PrototypeAst *Parser::parsePrototype() {
    if (_curTokenType != tok_func) {
        return Error("Expected 'func'.");
    }
    next(); // eat 'func'

    if (_curTokenType != tok_identifier) {
        return Error("Expected identifier in function prototype.");
    }
    std::string functionIdentifier = _curToken->Value();
    next(); // eat identifier

    if (_curTokenType != '(') {
        return Error("Expected '('.");
    }
    next(); // eat '('
    std::vector<std::pair<std::string, AstTypeNode*>> args;
    while (_curTokenType != ')') { // this while loop short circuits zero parameter functions
        if (_curTokenType != tok_identifier) {
            return Error("Expected identifier in parameter list.");
        }
        std::string argName = _curToken->Value();
        next(); // eat identifier
        if (_curTokenType != ':') {
            return Error("Expected ':' for parameter type definition.");
        }
        next(); // eat ':'
        AstTypeNode *argType = parseTypeNode();

        auto pair = std::make_pair(argName, argType);
        args.push_back(pair);
        if (_curTokenType != ',') {
            break;
        }
        next(); // eat ','
    }

    if (_curTokenType != ')') {
        return Error("Expected ')'.");
    }
    next(); // eat ')'

    if (_curTokenType != ':') {
        return Error("Expected ':' for function return type.");
    }
    next(); // eat ':'

    AstTypeNode *returnType = parseTypeNode();
    if (returnType == nullptr) {
        return Error("Expected function return type.");
    }

    return new PrototypeAst(functionIdentifier, returnType, args, false, _curToken->Line(), _curToken->Column());
}

// <extern>          ::= 'extern' 'func' id '(' (id ':')? <type> (',' (id ':')? <type>)* (',' '...')? ')' ':' <type>
PrototypeAst *Parser::parseExternDeclaration() {
    if (_curTokenType != tok_extern) {
        return Error("Expected 'extern'.");
    }
    next(); // eat 'extern'

    if (_curTokenType != tok_func) {
        return Error("Expected 'func'.");
    }
    next(); // eat 'func'

    if (_curTokenType != tok_identifier) {
        return Error("Expected function identifier in external declaration.");
    }
    std::string functionIdentifier = _curToken->Value();
    next(); // eat identifier

    if (_curTokenType != '(') {
        return Error("Expected '('.");
    }
    next(); // eat '('
    bool isVarArgs = false;
    std::vector<std::pair<std::string, AstTypeNode*>> args;
    int i = 0;
    while (_curTokenType != ')') { // this while loop short circuits zero parameter functions
        if (isVarArgs) {
            return Error("Varags, '...', must be at the end of parameter list.");
        }
        if (_curTokenType == tok_dotdotdot) { // found a varargs
            isVarArgs = true;
            next(); // eat '...'
        }
        else { // should be a type.
            if (_curTokenType == tok_identifier) // identifiers are optional in externs.
            {
                next(); // eat identifier
                if (_curTokenType != ':') {
                    return Error("Expected ':' after parameter name.");
                }
                next(); // eat ':'
            }
            AstTypeNode *argType = parseTypeNode();
            if (argType == nullptr) {
                return Error("Expected type identitifer in external declaration.");
            }
			std::string param = "param" + std::to_string(i);
            auto pair = std::make_pair(param, argType);
            args.push_back(pair);
        }
        if (_curTokenType != ',') {
            break;
        }
        next(); // eat ','
        i++;
    }

    if (_curTokenType != ')') {
        return Error("Expected ')'.");
    }
    next(); // eat ')'

    if (_curTokenType != ':') {
        return Error("Expected ':' for function return type.");
    }
    next(); // eat ':'

    AstTypeNode *returnType = parseTypeNode();
    if (returnType == nullptr) {
        return Error("Expected function return type.");
    }

    return new PrototypeAst(functionIdentifier, returnType, args, isVarArgs, _curToken->Line(), _curToken->Column());
}

ClassAst *Parser::parseClassDefinition() {
    if (_curTokenType != tok_class) {
        return Error("Expected 'class'.");
    }
    next(); // eat 'class'
    ClassAst *classAst = new ClassAst();

    if (_curTokenType != tok_identifier) {
        delete classAst;
        return Error("Expected class identifier.");
    }
    classAst->setName(_curToken->Value());
    next(); // eat identifier

    if (_curTokenType != '{') {
        delete classAst;
        return Error("Expected '{'.");
    }
    next(); // eat '{'

    while (_curTokenType != '}') {
        bool isPublic = _curTokenType == tok_public;
        if (_curTokenType == tok_private || isPublic) {
            next(); // eat 'public' or 'private'
        }
        if (_curTokenType == tok_func) { // Member function
            FunctionAst *func = parseFunctionDefinition();
            if (isPublic) { 
                classAst->pushPublicFunction(func);
            }
            else { // Assume that the default member type is 'private' even when not stated.
                classAst->pushPrivateFunction(func);
            }
        }
        else if (_curTokenType == tok_var) { // Member field
            AstVarExpr *varExpr = dynamic_cast<AstVarExpr*>(parseVarExpression());
            if (_curTokenType == ';') { next(); }
            if (isPublic) {
                classAst->pushPublicField(varExpr);
            }
            else { // Assume that the default member type is 'private' even when not stated.
                classAst->pushPrivateField(varExpr);
            }
        }
        else if (_curTokenType == tok_identifier) { // Possibly a constructor.
            if (_curToken->Value() != classAst->getName()) {
                delete classAst;
                return Error("Class constructor name must match the class name.");
            }
            next(); // eat identifier
            if (_curTokenType != '(') {
                return Error("Expected '('.");
            }
            next(); // eat '('
            std::vector<std::pair<std::string, AstTypeNode*>> args;
            while (_curTokenType != ')') { // this while loop short circuits zero parameter functions
                if (_curTokenType != tok_identifier) {
                    return Error("Expected identifier in parameter list.");
                }
                std::string argName = _curToken->Value();
                next(); // eat identifier
                if (_curTokenType != ':') {
                    return Error("Expected ':' for parameter type definition.");
                }
                next(); // eat ':'
                AstTypeNode *argType = parseTypeNode();

                auto pair = std::make_pair(argName, argType);
                args.push_back(pair);
                if (_curTokenType != ',') {
                    break;
                }
                next(); // eat ','
            }
            if (_curTokenType != ')') {
                return Error("Expected ')'.");
            }
            next(); // eat ')'

            if (_curTokenType != '{') {
                return Error("Expected '{'.");
            }
            next(); // eat '{'

            std::vector<IAstExpression*> functionBody;
            while (_curTokenType != '}') {
                IAstExpression *blockExpr = parseBlockExpression();
                if (blockExpr == nullptr) {
                    return Error("Unexpected token.");
                }
                functionBody.push_back(blockExpr);
            }
            next(); // eat '}'
            PrototypeAst *proto = new PrototypeAst(classAst->getName(), nullptr, args, false, _curToken->Line(), _curToken->Column());
            FunctionAst *ctor = new FunctionAst(proto, functionBody, _curToken->Line(), _curToken->Column());
            classAst->setConstructor(ctor);
        }
    }

    if (_curTokenType != '}') {
        delete classAst;
        return Error("Expected '}'.");
    }
    next(); // eat '}'
    return classAst;
}
