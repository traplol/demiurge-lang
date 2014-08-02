#include "AstReturnExpr.h"

#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"
#include "../DEFINES.h"

using namespace llvm;
Value *AstReturnExpr::Codegen(CodeGenerator *codegen) {
    Type *returnType = codegen->getBuilder().getCurrentFunctionReturnType();
    if (this->Expr == nullptr) // void return.
    {
        if (!returnType->isVoidTy()) // function return type not void, but trying to return void : error.
            return Helpers::Error(this->getPos(), "Cannot return void on function of return type '%s'", Helpers::GetLLVMTypeName(returnType).c_str());
        return codegen->getBuilder().CreateRetVoid();
    }
    Value *val = this->Expr->Codegen(codegen);
    if (val == nullptr)
        return Helpers::Error(this->getPos(), "Could not evaluate return statement.");
    Type *valType = val->getType();

    if (valType->getTypeID() != returnType->getTypeID()) {
        bool castSuccess;
        val = Helpers::CreateCastTo(codegen, val, returnType, &castSuccess);
        if (val == nullptr)
            return Helpers::Error(this->Expr->getPos(), "Could not cast return statement to function return type.");
        if (castSuccess) // warn that we automatically casted and that there might be a loss of data.
            Helpers::Warning(this->Expr->getPos(), "Casting from %s to %s, possible loss of data.",
            Helpers::GetLLVMTypeName(valType).c_str(), Helpers::GetLLVMTypeName(returnType).c_str());
    }
    AllocaInst *retVal = codegen->getNamedValue(COMPILER_RETURN_VALUE_STRING);
    codegen->getBuilder().CreateStore(val, retVal);

    Value *derefRetVal = codegen->getBuilder().CreateLoad(retVal, "retval");
    codegen->getBuilder().CreateRet(derefRetVal);
    return derefRetVal;
}
