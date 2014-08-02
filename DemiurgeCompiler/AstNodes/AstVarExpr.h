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
        int line, int column)
        : Name(name)
        , AssignmentExpression(assignmentExpression)
        , InferredType(inferredType) {
        setNodeType(node_var);
        setPos(PossiblePosition{ line, column });
    }
    ~AstVarExpr() {
        delete InferredType, AssignmentExpression;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    const std::string &getName() const { return Name; }
    AstTypeNode *getInferredType() const { return InferredType; }
    IAstExpression *getAssignmentExpression() const { return AssignmentExpression; }
};

#endif