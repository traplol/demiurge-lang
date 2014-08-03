#include "AstDoubleNode.h"

#include "../CodeGenerator/CodeGenerator.h"

using namespace llvm;

AstDoubleNode::AstDoubleNode(double value, int line, int column)
    : Val(value) {
    setNodeType(node_double);
    setPos(PossiblePosition{ line, column });
}

double AstDoubleNode::getValue() const { 
    return Val; 
}

Value *AstDoubleNode::Codegen(CodeGenerator *codegen) {
    return ConstantFP::get(codegen->getContext(), APFloat(this->Val));
}