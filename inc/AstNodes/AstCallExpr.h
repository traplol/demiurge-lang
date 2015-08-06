#ifndef _AST_CALL_EXPR_H
#define _AST_CALL_EXPR_H

#include "IAstExpression.h"
#include <vector>
#include <string>

class AstCallExpression : public IAstExpression {
    std::string Name;
    std::vector<IAstExpression*> Args;
public:
    AstCallExpression(const std::string &name, const std::vector<IAstExpression*> &args, int line, int column);
    ~AstCallExpression();
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    const std::string &getName() const;
};

#endif
