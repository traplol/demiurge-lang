#ifndef _AST_INTEGER_NODE_H
#define _AST_INTEGER_NODE_H

#include "IAstExpression.h"

class AstIntegerNode : public IAstExpression {
    // The reason this is unsigned is because rather than trying to parse a negative number
    // we will rely on the negate, '-', operator to create negative numbers.
    demi_int Val;
public:
    AstIntegerNode(demi_int value, int line, int column);
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    demi_int getValue() const;
};

#endif
