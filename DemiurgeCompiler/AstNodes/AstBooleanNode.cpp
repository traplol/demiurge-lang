#include "AstBooleanNode.h"
#include "../CodeGenerator/CodeGenerator.h"

using namespace llvm;
Value *AstBooleanNode::Codegen(CodeGenerator *codegen) {
    if (this->Val == true)
        return codegen->getBuilder().getTrue();
    return codegen->getBuilder().getFalse();
}
