#ifndef _AST_TYPE_NODE_H
#define _AST_TYPE_NODE_H

#include "AST_DEPENDENCIES.h"

class AstTypeNode {
    PossiblePosition Pos;
    AstNodeType TypeType;
public:
    AstTypeNode(AstNodeType type, int line, int column)
        : TypeType(type) {
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
    llvm::Type *GetLLVMType(CodeGenerator *codegen);
    PossiblePosition getPos() const { return Pos; }
    AstNodeType getTypeType() const { return TypeType; }
};

#endif