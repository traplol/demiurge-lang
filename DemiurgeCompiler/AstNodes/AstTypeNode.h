#ifndef _AST_TYPE_NODE_H
#define _AST_TYPE_NODE_H

#include "AST_DEPENDENCIES.h"
#include <string>
#include "IAstExpression.h"

class AstTypeNode {
    PossiblePosition Pos;
    AstNodeType TypeType;
    bool IsArray = false;
    demi_int ArraySize = 0;
    IAstExpression *Subscript;
    std::string TypeName;
public:
    AstTypeNode(AstNodeType type, const std::string &typeName, int line, int column);
    AstTypeNode(AstNodeType type, const std::string &typeName, bool isArray, IAstExpression *subscript, int line, int column);
    AstTypeNode(AstNodeType type, const std::string &typeName, bool isArray, demi_int arraySize, int line, int column);
    llvm::Type *GetLLVMType(CodeGenerator *codegen);
    PossiblePosition getPos() const;
    AstNodeType getTypeType() const;
    bool getIsArray() const;
    IAstExpression *getArraySubscript() const;
    demi_int getArraySize() const;
    std::string getTypeName() const;
private:
    void init(AstNodeType type, const std::string &typeName, bool isArray, demi_int arraySize, IAstExpression *subscript, int line, int column);
};

#endif