#ifndef _AST_VARIABLE_NODE_H
#define _AST_VARIABLE_NODE_H

#include "IAstExpression.h"
#include <string>

class AstVariableNode : public IAstExpression {
    std::string Name;
public:
    AstVariableNode(const std::string &name, int line, int column);
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    const std::string &getName() const;
};

#endif