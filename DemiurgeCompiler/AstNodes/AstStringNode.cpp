#include "AstStringNode.h"

#include "../CodeGenerator/CodeGenerator.h"

using namespace llvm;
Value *AstStringNode::Codegen(CodeGenerator *codegen) {
    return codegen->getBuilder().CreateGlobalStringPtr(this->Val, "globalstr_" + this->Val.substr(0, 10));
}