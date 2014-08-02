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
    FunctionAst(PrototypeAst *prototype, const std::vector<IAstExpression*> &functionBody, int line, int column)
        : Prototype(prototype)
        , FunctionBody(functionBody) {
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
    ~FunctionAst() {
        delete Prototype;
        while (!FunctionBody.empty()) delete FunctionBody.back(), FunctionBody.pop_back();
    }
    virtual llvm::Function *Codegen(CodeGenerator *codegen);
    PossiblePosition getPos() const { return Pos; }
    PrototypeAst *getPrototype() const { return Prototype; }
};

#endif