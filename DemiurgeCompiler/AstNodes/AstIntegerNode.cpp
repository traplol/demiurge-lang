#include "AstIntegerNode.h"

#include "../CodeGenerator/CodeGenerator.h"

using namespace llvm;

AstIntegerNode::AstIntegerNode(demi_int value, int line, int column)
    : Val(value) {
    setNodeType(node_unsigned_integer32);
    setPos(PossiblePosition{ line, column });
}
demi_int AstIntegerNode::getValue() const { 
    return Val; 
}

Value *AstIntegerNode::Codegen(CodeGenerator *codegen) {
    return ConstantInt::get(Type::getInt64Ty(codegen->getContext()), this->Val, true);
}
