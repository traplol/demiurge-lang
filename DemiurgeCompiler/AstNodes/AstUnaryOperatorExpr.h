#ifndef _AST_UNARY_OPERATOR_EXPR_H
#define _AST_UNARY_OPERATOR_EXPR_H

#include "IAstExpression.h"
#include <string>

class AstUnaryOperatorExpr : public IAstExpression {
    std::string OperatorString;
    TokenType Operator;
    IAstExpression *Operand;
    IAstExpression *IndexExpr;
    bool IsPostfix;
public:
    AstUnaryOperatorExpr(const std::string &operStr, TokenType oper, IAstExpression *operand, bool isPostfix, 
        IAstExpression *index, int line, int column);
    ~AstUnaryOperatorExpr();
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    virtual llvm::Value *ArrayAssignment(CodeGenerator *codegen, IAstExpression *rhs);
    virtual llvm::Value *positive(CodeGenerator *codegen);
    virtual llvm::Value *negative(CodeGenerator *codegen);
    virtual llvm::Value *logicNegate(CodeGenerator *codegen);
    virtual llvm::Value *onesComplement(CodeGenerator *codegen);
    virtual llvm::Value *increment(CodeGenerator *codegen);
    virtual llvm::Value *decrement(CodeGenerator *codegen);
    virtual llvm::Value *accessElement(CodeGenerator *codegen);

    bool getIsPostfix() const;
    bool getIsPrefix() const;
};

#endif