#ifndef _AST_INTEGER_NODE_H
#define _AST_INTEGER_NODE_H

#include "IAstExpression.h"

class AstIntegerNode : public IAstExpression {
    // The reason this is unsigned is because rather than trying to parse a negative number
    // we will rely on the negate, '-', operator to create negative numbers.
    unsigned long long int Val;
public:
    AstIntegerNode(unsigned long long int value, int line, int column)
        : Val(value) {
        setNodeType(node_integer);
        setPos(PossiblePosition{ line, column });
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    unsigned long long int getValue() const { return Val; }
};

#endif