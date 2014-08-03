#ifndef _AST_TYPE_NODE_H
#define _AST_TYPE_NODE_H

#include "AST_DEPENDENCIES.h"
#include <string>

class AstTypeNode {
    PossiblePosition Pos;
    AstNodeType TypeType;
    bool IsArray;
    demi_int ArraySize;
    std::string TypeName;
public:
    AstTypeNode(AstNodeType type, const std::string &typeName, int line, int column, bool isArray = false,
        demi_int arraySize = 0);
    llvm::Type *GetLLVMType(CodeGenerator *codegen);
    PossiblePosition getPos() const;
    AstNodeType getTypeType() const;
    bool getIsArray() const;
    demi_int getArraySize() const;
    std::string getTypeName() const;
};

#endif