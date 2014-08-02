#ifndef _AST_CALL_EXPR_H
#define _AST_CALL_EXPR_H

#include "IAstExpression.h"
#include <vector>
#include <string>

class AstCallExpression : public IAstExpression {
    std::string Name;
    std::vector<IAstExpression*> Args;
public:
    AstCallExpression(const std::string &name, const std::vector<IAstExpression*> &args, int line, int column)
        : Name(name)
        , Args(args) {
        setNodeType(node_call);
        setPos(PossiblePosition{ line, column });
    }
    ~AstCallExpression() {
        while (!Args.empty()) delete Args.back(), Args.pop_back();
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    const std::string &getName() const { return Name; }
};

#endif