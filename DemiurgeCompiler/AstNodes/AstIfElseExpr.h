#ifndef _AST_IF_ELSE_EXPR_H
#define _AST_IF_ELSE_EXPR_H

#include "IAstExpression.h"
#include <vector>

class AstIfElseExpr : public IAstExpression {
    IAstExpression *Condition;
    std::vector<IAstExpression*> IfBody, ElseBody;
public:
    AstIfElseExpr(IAstExpression *condition, const std::vector<IAstExpression*> &ifBody,
        const std::vector<IAstExpression*> &elseBody, int line, int column)
        : Condition(condition)
        , IfBody(ifBody)
        , ElseBody(elseBody) {
        setNodeType(node_ifelse);
        setPos(PossiblePosition{ line, column });
    }
    ~AstIfElseExpr() {
        delete Condition;
        while (!IfBody.empty()) delete IfBody.back(), IfBody.pop_back();
        while (!ElseBody.empty()) delete ElseBody.back(), ElseBody.pop_back();
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

#endif