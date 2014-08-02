#include "AstIntegerNode.h"

#include "../CodeGenerator/CodeGenerator.h"

using namespace llvm;
Value *AstIntegerNode::Codegen(CodeGenerator *codegen) {
    return ConstantInt::get(Type::getInt64Ty(codegen->getContext()), this->Val, true);
}
