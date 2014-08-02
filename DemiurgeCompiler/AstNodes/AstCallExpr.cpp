#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

#include "AstCallExpr.h"
#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"


using namespace llvm;
Value *AstCallExpression::Codegen(CodeGenerator *codegen) {
    // Lookup the name in the global module table.
    Function *CalleeF = codegen->getTheModule()->getFunction(this->Name);
    if (CalleeF == nullptr)
        return Helpers::Error(this->getPos(), "Unknown function, '%s', referenced.", this->Name.c_str());

    // If argument count is mismatched
    if (CalleeF->arg_size() != Args.size() && !CalleeF->isVarArg())
        return Helpers::Error(this->getPos(), "Incorrect number of arguments passed to function '%s'", this->Name.c_str());
    std::vector<Value*> argsvals;
    auto calleeArg = CalleeF->arg_begin();
    for (unsigned i = 0, len = this->Args.size(); i < len; ++i, ++calleeArg){
        Value *val = this->Args[i]->Codegen(codegen);
        if (val == nullptr)
            return Helpers::Error(this->Args[i]->getPos(), "Function argument could not be evaulated.");
        bool castSuccess = false;
        if (!CalleeF->isVarArg()) // don't try to cast varargs.
            val = Helpers::CreateCastTo(codegen, val, calleeArg->getType(), &castSuccess);
        if (val == nullptr)
            return Helpers::Error(this->Args[i]->getPos(), "Function argument not valid type, failed to cast to destination type.");
        if (castSuccess) // warn that we automatically casted and that there might be a loss of data.
            Helpers::Warning(this->Args[i]->getPos(), "Casting from %s to %s, possible loss of data.",
            Helpers::GetLLVMTypeName(val->getType()).c_str(), Helpers::GetLLVMTypeName(calleeArg->getType()).c_str());
        argsvals.push_back(val);
    }
    bool isVoidReturn = CalleeF->getReturnType()->isVoidTy();
    return codegen->getBuilder().CreateCall(CalleeF, argsvals, isVoidReturn ? "" : "calltmp");
}