#ifndef _AST_BOOLEAN_NODE_H
#define _AST_BOOLEAN_NODE_H

#include "IAstExpression.h"

class AstBooleanNode : public IAstExpression {
    bool Val;
public:
    AstBooleanNode(bool value, int line, int column)
        : Val(value) {
        setNodeType(node_boolean);
        setPos(PossiblePosition{ line, column });
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    bool getValue() const { return Val; }
};

#endif