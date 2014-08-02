#ifndef _AST_RETURN_EXPR_H
#define _AST_RETURN_EXPR_H

#include "IAstExpression.h"

class AstReturnExpr : public IAstExpression {
    IAstExpression *Expr;
public:
    AstReturnExpr(IAstExpression *expr, int line, int column)
        : Expr(expr) {
        setNodeType(node_return);
        setPos(PossiblePosition{ line, column });
    }
    ~AstReturnExpr() {
        delete Expr;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

#endif