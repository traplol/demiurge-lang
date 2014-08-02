#include "AstTypeNode.h"

#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"

using namespace llvm;
Type *AstTypeNode::GetLLVMType(CodeGenerator *codegen) {
    switch (this->TypeType) {
    default: Helpers::Error(this->getPos(), "Unknown type.");
    case node_boolean: return Type::getInt1Ty(codegen->getContext());
    case node_double: return Type::getDoubleTy(codegen->getContext());
    case node_integer: return Type::getInt64Ty(codegen->getContext());
    case node_string: return Type::getInt8PtrTy(codegen->getContext());
    case node_void: return Type::getVoidTy(codegen->getContext());
    }
}