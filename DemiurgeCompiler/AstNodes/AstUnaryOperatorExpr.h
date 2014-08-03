#ifndef _AST_UNARY_OPERATOR_EXPR_H
#define _AST_UNARY_OPERATOR_EXPR_H

#include "IAstExpression.h"
#include <string>

class AstUnaryOperatorExpr : public IAstExpression {
    std::string OperatorString;
    TokenType Operator;
    IAstExpression *Operand;
    bool IsPostfix;
public:
    AstUnaryOperatorExpr(const std::string &operStr, TokenType oper, IAstExpression *operand, bool isPostfix, int line, int column)
        : OperatorString(operStr)
        , Operator(oper)
        , Operand(operand)
        , IsPostfix(isPostfix){
        setNodeType(node_binary_operation);
        setPos(PossiblePosition{ line, column });
    }
    ~AstUnaryOperatorExpr() {
        delete Operand;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    virtual llvm::Value *VariableAssignment(CodeGenerator *codegen);
};

#endif