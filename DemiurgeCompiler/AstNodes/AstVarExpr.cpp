#include "AstVarExpr.h"

#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"

using namespace llvm;

Value *AstVarExpr::Codegen(CodeGenerator *codegen) {
    std::vector<AllocaInst*> oldBindings;
    Function *func = codegen->getBuilder().GetInsertBlock()->getParent();

    IAstExpression *expr = this->getAssignmentExpression();
    Value *initialVal;
    AllocaInst *Alloca;
    if (expr == nullptr) { // variable declaration without an assignment, e.g: 'var x : int;'
        initialVal = Helpers::GetDefaultValue(codegen, this->InferredType);
        Alloca = Helpers::CreateEntryBlockAlloca(func, this->Name.c_str(), this->InferredType->GetLLVMType(codegen));
    }
    else {
        initialVal = expr->Codegen(codegen);
        auto type = Helpers::GetLLVMTypeName(initialVal->getType());
        Alloca = Helpers::CreateEntryBlockAlloca(func, this->Name, initialVal->getType());
    }
    codegen->getBuilder().CreateStore(initialVal, Alloca);
    codegen->setNamedValue(this->Name, Alloca);
    codegen->incrementVarCount();
    return initialVal;
}
