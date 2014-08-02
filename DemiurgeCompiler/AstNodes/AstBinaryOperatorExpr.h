#ifndef _AST_BINARY_OPERATOR_EXPR_H
#define _AST_BINARY_OPERATOR_EXPR_H

#include "IAstExpression.h"
#include <string>

class AstBinaryOperatorExpr : public IAstExpression {
    std::string OperatorString;
    TokenType Operator;
    IAstExpression *LHS, *RHS;
public:
    AstBinaryOperatorExpr(const std::string &operStr, TokenType oper, IAstExpression *lhs,
        IAstExpression *rhs, int line, int column)
        : OperatorString(operStr)
        , Operator(oper)
        , LHS(lhs)
        , RHS(rhs) {
        setNodeType(node_binary_operation);
        setPos(PossiblePosition{ line, column });
    }
    ~AstBinaryOperatorExpr() {
        delete LHS, RHS;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    virtual llvm::Value *VariableAssignment(CodeGenerator *codegen);
};

#endif