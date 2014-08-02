#ifndef _AST_DOUBLE_NODE_H
#define _AST_DOUBLE_NODE_H

#include "IAstExpression.h"

class AstDoubleNode : public IAstExpression {
    double Val;
public:
    AstDoubleNode(double value, int line, int column)
        : Val(value) {
        setNodeType(node_double);
        setPos(PossiblePosition{ line, column });
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    double getValue() const { return Val; }
};

#endif