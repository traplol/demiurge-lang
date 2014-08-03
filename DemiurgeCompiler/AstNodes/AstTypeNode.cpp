#include "AstTypeNode.h"

#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"

using namespace llvm;

AstTypeNode::AstTypeNode(AstNodeType type, const std::string &typeName, int line, int column, bool isArray,
    demi_int arraySize)
    : TypeType(type)
    , IsArray(isArray)
    , TypeName(typeName)
    , ArraySize(arraySize) {
    Pos.LineNumber = line;
    Pos.ColumnNumber = column;
}
PossiblePosition AstTypeNode::getPos() const {
    return Pos; 
}
AstNodeType AstTypeNode::getTypeType() const {
    return TypeType; 
}
bool AstTypeNode::getIsArray() const {
    return IsArray; 
}
demi_int AstTypeNode::getArraySize() const {
    return ArraySize; 
}
std::string AstTypeNode::getTypeName() const {
    return TypeName; 
}

Type *AstTypeNode::GetLLVMType(CodeGenerator *codegen) {
    Type *type;
    switch (this->TypeType) {
    default: return Helpers::Error(this->getPos(), "Unknown type.");
    case node_boolean: type = Type::getInt1Ty(codegen->getContext()); break;
    case node_double: type = Type::getDoubleTy(codegen->getContext()); break;
    case node_integer: type = Type::getInt64Ty(codegen->getContext()); break;
    case node_string: type = Type::getInt8PtrTy(codegen->getContext()); break;
    case node_void: type = Type::getVoidTy(codegen->getContext()); break;
    }
    if (this->IsArray && this->ArraySize > 0)
        return ArrayType::get(type, this->ArraySize);
    if (this->IsArray && this->ArraySize == 0) {
        return PointerType::get(type, 0);
    }
    return type;
}