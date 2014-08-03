#include "AstStringNode.h"

#include "../CodeGenerator/CodeGenerator.h"

using namespace llvm;

AstStringNode::AstStringNode(const std::string &value, int line, int column)
    : Val(value) {
    setNodeType(node_string);
    setPos(PossiblePosition{ line, column });
}

const std::string &AstStringNode::getValue() const { 
    return Val; 
}

Value *AstStringNode::Codegen(CodeGenerator *codegen) {
    return codegen->getBuilder().CreateGlobalStringPtr(this->Val, "globalstr_" + this->Val.substr(0, 10));
}