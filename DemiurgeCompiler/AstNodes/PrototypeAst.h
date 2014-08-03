#ifndef _PROTOTYPE_AST_H
#define _PROTOTYPE_AST_H

#include "AstTypeNode.h"
#include "IAstExpression.h"
#include <vector>
#include <string>

class PrototypeAst {
    PossiblePosition Pos;
    std::string Name;
    std::vector<std::pair<std::string, AstTypeNode*>> Args;
    AstTypeNode *ReturnType;
    bool IsVarArgs;
public:
    PrototypeAst(const std::string &name, AstTypeNode *returnType, const std::vector<std::pair<std::string, AstTypeNode*>> &args,
        bool isVarArgs, int line, int column);
    ~PrototypeAst();
    virtual llvm::Function *Codegen(CodeGenerator *codegen);
    void CreateArgumentAllocas(CodeGenerator *codegen, llvm::Function *func);

    PossiblePosition getPos() const;
    const std::string &getName() const;
    AstTypeNode *getReturnType() const;
};

#endif