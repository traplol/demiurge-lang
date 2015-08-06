#ifndef _AST_VAR_EXPR_H
#define _AST_VAR_EXPR_H

#include "IAstExpression.h"
#include "AstTypeNode.h"
#include <string>

class AstVarExpr : public IAstExpression {
    std::string Name;
    AstTypeNode *InferredType;
    IAstExpression *AssignmentExpression;
public:
    AstVarExpr(const std::string &name, IAstExpression *assignmentExpression, AstTypeNode *inferredType,
        int line, int column);
    ~AstVarExpr();
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    const std::string &getName() const;
    AstTypeNode *getInferredType() const;
    IAstExpression *getAssignmentExpression() const;
};

#endif