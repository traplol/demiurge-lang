#include "AstVarExpr.h"

#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"

using namespace llvm;

Value *AstVarExpr::Codegen(CodeGenerator *codegen) {
    codegen->incrementVarCount();

    std::vector<AllocaInst*> oldBindings;
    Function *func = codegen->getBuilder().GetInsertBlock()->getParent();
    IAstExpression *expr = this->getAssignmentExpression();
    Value *initialVal = nullptr;
    AllocaInst *Alloca = nullptr;
    if (expr == nullptr) { // variable declaration without an assignment, e.g: 'var x : int;'
        if (this->InferredType->getIsArray()) { // type is an array.
            Type *arrayType = this->InferredType->GetLLVMType(codegen);
            Alloca = Helpers::CreateEntryBlockAlloca(codegen, func, this->Name.c_str(), arrayType);
        }
        else {
            initialVal = Helpers::GetDefaultValue(codegen, this->InferredType);
            Alloca = Helpers::CreateEntryBlockAlloca(codegen, func, this->Name.c_str(), this->InferredType->GetLLVMType(codegen));
        }
    }
    else {
        initialVal = expr->Codegen(codegen);
        auto type = Helpers::GetLLVMTypeName(initialVal->getType());
        Alloca = Helpers::CreateEntryBlockAlloca(codegen, func, this->Name, initialVal->getType());
    }
    if (initialVal != nullptr) {
        codegen->getBuilder().CreateStore(initialVal, Alloca);
    }
    codegen->setNamedValue(this->Name, Alloca);
    return initialVal == nullptr ? Alloca : initialVal;
}
