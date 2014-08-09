#include "AstVariableNode.h"

#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"

using namespace llvm;

AstVariableNode::AstVariableNode(const std::string &name, int line, int column)
    : Name(name) {
    setNodeType(node_variable);
    setPos(PossiblePosition{ line, column });
}

const std::string &AstVariableNode::getName() const { 
    return Name; 
}

Value *AstVariableNode::Codegen(CodeGenerator *codegen) {
    Value *v = codegen->getNamedValue(this->Name);
    if (v == nullptr) {
        return Helpers::Error(this->getPos(), "Unknown variable name.");
    }
    if (Helpers::IsPtrToPtr(v)) {
        // TODO:
    }
    else if (Helpers::IsPtrToArray(v)) {
        //Value *arr[] = { Helpers::GetZero_64(codegen), Helpers::GetZero_64(codegen) };
        //return codegen->getBuilder().CreateGEP(v, arr, "arraydecay");
        return v;
    }
    auto type = Helpers::GetLLVMTypeName(v->getType());
    return codegen->getBuilder().CreateLoad(v, this->Name.c_str());
}
