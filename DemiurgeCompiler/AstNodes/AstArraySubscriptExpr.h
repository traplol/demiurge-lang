#ifndef _AST_ARRAY_SUBSCRIPT_EXPR_H
#define _AST_ARRAY_SUBSCRIPT_EXPR_H

#include "IAstExpression.h"

class AstArraySubscriptExpr : public IAstExpression {
    IAstExpression *Subscript;
public:
    AstArraySubscriptExpr(IAstExpression *subscript, int line, int column)
        : Subscript(subscript) {
        setNodeType(node_array_subscript);
        setPos(PossiblePosition{ line, column });
    }
    ~AstArraySubscriptExpr() {
        delete Subscript;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    virtual llvm::Value *Assignment(CodeGenerator *codegen);
};

#endif