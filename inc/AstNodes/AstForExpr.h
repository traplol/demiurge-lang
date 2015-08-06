#ifndef _AST_FOR_EXPR_H
#define _AST_FOR_EXPR_H

#include "IAstExpression.h"
#include <vector>

class AstForExpr : public IAstExpression {
    IAstExpression *Condition;
    std::vector<IAstExpression*> Init, Afterthought, Body;
public:
    AstForExpr(const std::vector<IAstExpression*> &init, IAstExpression *condition,
        const std::vector<IAstExpression*> &afterThought, const std::vector<IAstExpression*> &body,
        int line, int column);
    ~AstForExpr();
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

#endif
