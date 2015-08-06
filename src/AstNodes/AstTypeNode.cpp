#include "AstNodes/AstTypeNode.h"

#include "CodeGenerator/CodeGenerator.h"
#include "CodeGenerator/CodeGeneratorHelpers.h"

using namespace llvm;

AstTypeNode::AstTypeNode(AstNodeType type, const std::string &typeName, int line, int column) {
    init(type, typeName, false, 0, nullptr, line, column);
}

AstTypeNode::AstTypeNode(AstNodeType type, const std::string &typeName, bool isArray, IAstExpression *subscript, int line, int column) {
    init(type, typeName, true, 0, subscript, line, column);
}

AstTypeNode::AstTypeNode(AstNodeType type, const std::string &typeName, bool isArray, demi_int arraySize, int line, int column) {
    init(type, typeName, true, arraySize, nullptr, line, column);
}

void AstTypeNode::init(AstNodeType type, const std::string &typeName, bool isArray, demi_int arraySize, IAstExpression *subscript, int line, int column) {
    Pos.LineNumber = line;
    Pos.ColumnNumber = column;
    TypeType = type;
    IsArray = isArray;
    TypeName = typeName;
    ArraySize = arraySize;
    Subscript = subscript;
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
IAstExpression *AstTypeNode::getArraySubscript() const {
    return Subscript;
}
std::string AstTypeNode::getTypeName() const {
    return TypeName; 
}

Type *AstTypeNode::GetLLVMType(CodeGenerator *codegen) {
    Type *type;
    switch (this->TypeType) {
    default: return Helpers::Error(this->getPos(), "Unknown type.");
    
    case node_double: type = Type::getDoubleTy(codegen->getContext()); break;
    case node_float: type = Type::getFloatTy(codegen->getContext()); break;
    
    case node_boolean: type = Type::getInt1Ty(codegen->getContext()); break;
    case node_signed_integer8: type = Type::getInt8Ty(codegen->getContext()); break;
    case node_signed_integer16: type = Type::getInt16Ty(codegen->getContext()); break;
    case node_signed_integer32: type = Type::getInt32Ty(codegen->getContext()); break;
    case node_signed_integer64: type = Type::getInt64Ty(codegen->getContext()); break;

    case node_unsigned_integer8: type = Type::getInt8Ty(codegen->getContext()); break;
    case node_unsigned_integer16: type = Type::getInt16Ty(codegen->getContext()); break;
    case node_unsigned_integer32: type = Type::getInt32Ty(codegen->getContext()); break;
    case node_unsigned_integer64: type = Type::getInt64Ty(codegen->getContext()); break;

    case node_string: type = Type::getInt8PtrTy(codegen->getContext()); break;
    case node_void: type = Type::getVoidTy(codegen->getContext()); break;
    }
    if (this->IsArray && this->ArraySize > 0) {
        return ArrayType::get(type, this->ArraySize); // note this is array type
    }
    if (this->IsArray && this->ArraySize == 0) {
        return PointerType::get(type, 0); // note this is ptr type
    }
    return type;
}
