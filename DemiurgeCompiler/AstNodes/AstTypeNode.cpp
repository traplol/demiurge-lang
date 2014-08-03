#include "AstTypeNode.h"

#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"

using namespace llvm;
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