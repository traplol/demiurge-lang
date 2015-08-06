#ifndef _I_AST_EXPRESSION_H
#define _I_AST_EXPRESSION_H

#include "AST_DEPENDENCIES.h"

class IAstExpression {
    PossiblePosition Pos;
    AstNodeType NodeType;
public:
    IAstExpression() : NodeType(node_default){}
    virtual ~IAstExpression(){}
    virtual llvm::Value *Codegen(CodeGenerator *codegen) = 0;
    void setNodeType(AstNodeType type) { NodeType = type; }
    void setPos(PossiblePosition pos) { Pos = pos; }
    AstNodeType getNodeType() const { return NodeType; }
    PossiblePosition getPos() const { return Pos; }
};

#endif