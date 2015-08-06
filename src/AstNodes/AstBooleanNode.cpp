#include "AstNodes/AstBooleanNode.h"
#include "CodeGenerator/CodeGenerator.h"

using namespace llvm;

AstBooleanNode::AstBooleanNode(bool value, int line, int column)
    : Val(value) {
    setNodeType(node_boolean);
    setPos(PossiblePosition{ line, column });
}
bool AstBooleanNode::getValue() const { 
    return Val; 
}


Value *AstBooleanNode::Codegen(CodeGenerator *codegen) {
    if (this->Val == true) {
        return codegen->getBuilder().getTrue();
    }
    return codegen->getBuilder().getFalse();
}
