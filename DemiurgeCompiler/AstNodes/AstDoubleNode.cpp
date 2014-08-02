#include "AstDoubleNode.h"

#include "../CodeGenerator/CodeGenerator.h"

using namespace llvm;
Value *AstDoubleNode::Codegen(CodeGenerator *codegen) {
    return ConstantFP::get(codegen->getContext(), APFloat(this->Val));
}