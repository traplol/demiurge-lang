#ifndef _AST_BOOLEAN_NODE_H
#define _AST_BOOLEAN_NODE_H

#include "IAstExpression.h"

class AstBooleanNode : public IAstExpression {
    bool Val;
public:
    AstBooleanNode(bool value, int line, int column);
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    bool getValue() const;
};

#endif
