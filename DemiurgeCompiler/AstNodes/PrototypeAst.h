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
        bool isVarArgs, int line, int column)
        : Name(name)
        , ReturnType(returnType)
        , Args(args)
        , IsVarArgs(isVarArgs) {
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
    ~PrototypeAst() {
        delete ReturnType;
        for (auto begin = Args.begin(); begin != Args.end(); ++begin) {
            delete begin->second;
        }
    }
    virtual llvm::Function *Codegen(CodeGenerator *codegen);
    void CreateArgumentAllocas(CodeGenerator *codegen, llvm::Function *func);

    PossiblePosition getPos() const { return Pos; }
    const std::string &getName() const { return Name; }
    AstTypeNode *getReturnType() const { return ReturnType; }
};

#endif