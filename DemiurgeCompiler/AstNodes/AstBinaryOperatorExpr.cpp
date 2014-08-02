//#include "llvm/IR/Type.h"
//#include "llvm/IR/Value.h"

#include "AstBinaryOperatorExpr.h"
#include "AstVariableNode.h"
#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"

using namespace llvm;

Value *AstBinaryOperatorExpr::VariableAssignment(CodeGenerator *codegen) {
    AstVariableNode *lhse = dynamic_cast<AstVariableNode*>(this->LHS);
    if (lhse == nullptr) // left side of assign operator isn't a variable.
        return Helpers::Error(this->LHS->getPos(), "Destination of assignment operator must be variable.");

    // Codegen to evaluate the expression to store within the variable.
    Value *val = this->RHS->Codegen(codegen);
    if (val == nullptr) return Helpers::Error(this->RHS->getPos(), "Right operand could not be evaluated.");

    // Lookup the name of the variable
    
    Value *variable = codegen->getNamedValue(lhse->getName());
    if (variable == nullptr) return Helpers::Error(this->getPos(), "Unknown variable name '%s'", lhse->getName().c_str());

    // Implicit casting to destination type when assigning to variables.
    Type *varType = variable->getType()->getContainedType(0);
    val = Helpers::CreateCastTo(codegen, val, varType);

    codegen->getBuilder().CreateStore(val, variable);
    return val;
}
Value *AstBinaryOperatorExpr::Codegen(CodeGenerator *codegen) {
    if (this->Operator == '=') {
        return this->VariableAssignment(codegen);
    }
    Value *l = this->LHS->Codegen(codegen);
    Value *r = this->RHS->Codegen(codegen);
    if (l == nullptr || r == nullptr) return Helpers::Error(this->getPos(), "Could not evaluate expression!");
    Type *lType = l->getType();
    Type *rType = r->getType();
    auto funcPtr = Helpers::GetBinopCodeGenFuncPointer(this->Operator, lType, rType);
    if (funcPtr == nullptr) {
        return Helpers::Error(this->getPos(), "Operator '%s' does not exist for '%s' and '%s'",
            this->OperatorString.c_str(), Helpers::GetLLVMTypeName(lType).c_str(), Helpers::GetLLVMTypeName(rType).c_str());
    }
    return funcPtr(codegen, l, r);
}
