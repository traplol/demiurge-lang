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
        demi_int arraySize = 0)
        : TypeType(type)
        , IsArray(isArray)
        , TypeName(typeName)
        , ArraySize(arraySize) {
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
    llvm::Type *GetLLVMType(CodeGenerator *codegen);
    PossiblePosition getPos() const { return Pos; }
    AstNodeType getTypeType() const { return TypeType; }
    bool getIsArray() const { return IsArray; }
    demi_int getArraySize() const { return ArraySize; }
    std::string getTypeName() const { return TypeName; }
};

#endif