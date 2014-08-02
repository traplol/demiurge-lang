#ifndef _AST_STRING_NODE_H
#define _AST_STRING_NODE_H

#include "IAstExpression.h"
#include <string>

class AstStringNode : public IAstExpression {
    std::string Val;
public:
    AstStringNode(const std::string &value, int line, int column)
        : Val(value) {
        setNodeType(node_string);
        setPos(PossiblePosition{ line, column });
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    const std::string &getValue() const { return Val; }
};

#endif