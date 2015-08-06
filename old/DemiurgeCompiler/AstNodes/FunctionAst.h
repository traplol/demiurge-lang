#ifndef _FUNCTION_AST_H
#define _FUNCTION_AST_H

#include "IAstExpression.h"
#include "PrototypeAst.h"
#include <vector>

class FunctionAst {
    PossiblePosition Pos;
    PrototypeAst *Prototype;
    std::vector<IAstExpression*> FunctionBody;
public:
    FunctionAst(PrototypeAst *prototype, const std::vector<IAstExpression*> &functionBody, int line, int column);
    ~FunctionAst();
    virtual llvm::Function *Codegen(CodeGenerator *codegen);
    PossiblePosition getPos() const;
    PrototypeAst *getPrototype() const;
};

#endif