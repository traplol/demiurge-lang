#include "AstVariableNode.h"

#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"

using namespace llvm;

Value *AstVariableNode::Codegen(CodeGenerator *codegen) {
    Value *v = codegen->getNamedValue(this->Name);
    if (v == nullptr) return Helpers::Error(this->getPos(), "Unknown variable name.");
    auto type = Helpers::GetLLVMTypeName(v->getType());
    return codegen->getBuilder().CreateLoad(v, this->Name.c_str());
}
