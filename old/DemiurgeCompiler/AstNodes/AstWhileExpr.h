#ifndef _AST_WHILE_EXPR_H
#define _AST_WHILE_EXPR_H

#include "IAstExpression.h"
#include <vector>

class AstWhileExpr : public IAstExpression {
    IAstExpression *Condition;
    std::vector<IAstExpression*> WhileBody;
public:
    AstWhileExpr(IAstExpression *condition, const std::vector<IAstExpression*> &whileBody,
        int line, int column);
    ~AstWhileExpr();
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

#endif