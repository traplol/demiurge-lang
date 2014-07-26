#include "llvm/Analysis/Passes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include <cctype>
#include <cstdio>
#include <stdarg.h>
#include <map>

#include "CodeGenerator.h"
#include "CodeGeneratorHelpers.h"
#include "ASTNodes.h"
#include "TreeContainer.h"

using namespace llvm;

#define COMPILER_RETURN_VALUE_STRING "__return_value__"

namespace Helpers {
    //static nullptr_t Error(const char *fmt, ...);
}

CodeGenerator::CodeGenerator() 
    : Context(getGlobalContext())
    , Builder(getGlobalContext()) {
    InitializeNativeTarget();
    TheModule = new Module("Test Module", Context);

    // Create the JIT.  This takes ownership of the module.
    std::string ErrStr;
    TheExecutionEngine = EngineBuilder(TheModule).setErrorStr(&ErrStr).create();
    if (!TheExecutionEngine) {
        fprintf(stderr, "Could not create ExecutionEngine: %s\n", ErrStr.c_str());
        exit(1);
    }

    TheFPM = new FunctionPassManager(TheModule);

    // Set up the optimizer pipeline.  Start with registering info about how the
    // target lays out data structures.
    TheModule->setDataLayout(TheExecutionEngine->getDataLayout());
    TheFPM->add(new DataLayoutPass(TheModule));
    // Provide basic AliasAnalysis support for GVN.
    TheFPM->add(createBasicAliasAnalysisPass());
    // Promote allocas to registers.
    TheFPM->add(createPromoteMemoryToRegisterPass());
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->add(createInstructionCombiningPass());
    // Reassociate expressions.
    TheFPM->add(createReassociatePass());
    // Eliminate Common SubExpressions.
    TheFPM->add(createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    TheFPM->add(createCFGSimplificationPass());

    TheFPM->doInitialization();
}


CodeGenerator::~CodeGenerator() {
}

void CodeGenerator::GenerateCode(TreeContainer *trees) {
    for (int i = 0, e = trees->FunctionDefinitions.size(); i < e; ++i) {
        trees->FunctionDefinitions[i]->Codegen(this);
    }
    for (int i = 0, e = trees->TopLevelExpressions.size(); i < e; ++i) {
        trees->TopLevelExpressions[i]->Codegen(this);
    }
}

void CodeGenerator::CacheLastModule() {

}

void CodeGenerator::DumpLastModule() {
    auto moduleName = TheModule->getModuleIdentifier().c_str();
    fprintf(stderr, "\n\n============ BEGIN DUMPING MODULE '%s' ============\n\n", moduleName);
    TheModule->dump();
    fprintf(stderr, "\n\n============ END DUMPING MODULE '%s' ============\n\n", moduleName);
    
    Function *mainFunc = TheModule->getFunction("main");
    if (mainFunc == nullptr) {
        Helpers::Error(PossiblePosition{-1,-1}, "No main function found!");
        return;
    }
    void *mainFnPtr = TheExecutionEngine->getPointerToFunction(mainFunc);
    
    int(*FP)() = (int(*)())mainFnPtr;
    
    int val = FP();

    printf("%f\n",(double)val);
}

Type *AstTypeNode::GetLLVMType(CodeGenerator *codegen) {
    switch (this->TypeType) {
    default: return nullptr;
    case node_boolean: return Type::getInt1Ty(codegen->Context);
    case node_double: return Type::getDoubleTy(codegen->Context);
    case node_integer: return Type::getInt64Ty(codegen->Context);
    case node_string: return Type::getInt8PtrTy(codegen->Context);
    case node_void: return Type::getVoidTy(codegen->Context);
    }
}

Value *AstDoubleNode::Codegen(CodeGenerator *codegen) {
    return ConstantFP::get(codegen->Context, APFloat(this->Value));
}
Value *AstIntegerNode::Codegen(CodeGenerator *codegen) {
    return ConstantInt::get(codegen->Context, APInt(64, this->Value));
}
Value *AstBooleanNode::Codegen(CodeGenerator *codegen) {
    return ConstantInt::get(codegen->Context, APInt(this->Value, 1));
}
Value *AstStringNode::Codegen(CodeGenerator *codegen) {
    return ConstantDataArray::getString(codegen->Context, this->Value.c_str());
}
Value *AstBinaryOperatorExpr::Codegen(CodeGenerator *codegen) {
    Value *l = this->LHS->Codegen(codegen);
    Value *r = this->RHS->Codegen(codegen);
    if (l == nullptr || r == nullptr) return nullptr;
    Type *lType = l->getType();
    Type *rType = r->getType();
    auto funcPtr = Helpers::GetBinopCodeGenFuncPointer(this->Operator, lType, rType);
    if (funcPtr == nullptr) {
        return Helpers::Error(this->Pos, "Operator '%s' does not exist for '%s' and '%s'",
            this->OperatorString.c_str(), Helpers::GetLLVMTypeName(lType).c_str(), Helpers::GetLLVMTypeName(rType).c_str());
    }
    return funcPtr(codegen, l, r);
}
Value *AstReturnNode::Codegen(CodeGenerator *codegen) { 
    // TODO: returning void.
    Value *val = this->Expr->Codegen(codegen);
    if (val == nullptr)
        return Helpers::Error(this->Pos, "Could not evaluate return statement.");
    Type *valType = val->getType();
    Type *retType = codegen->Builder.getCurrentFunctionReturnType();
    if (valType->getTypeID() != retType->getTypeID()) {
        bool castSuccess;
        val = Helpers::CreateCastTo(codegen, val, retType, &castSuccess);
        if (val == nullptr) 
            return Helpers::Error(this->Expr->Pos, "Could not cast return statement to function return type.");
        if (castSuccess) // warn that we automatically casted and that there might be a loss of data.
            Helpers::Warning(this->Expr->Pos, "Casting from %s to %s, possible loss of data.",
                Helpers::GetLLVMTypeName(valType).c_str(), Helpers::GetLLVMTypeName(retType).c_str());
    }
    AllocaInst *retVal = codegen->NamedValues[COMPILER_RETURN_VALUE_STRING];
    Value *derefRetVal = codegen->Builder.CreateStore(val, retVal);
    codegen->Builder.CreateBr(codegen->ReturnBlock);
    return derefRetVal;
}
Value *AstIfElseExpression::Codegen(CodeGenerator *codegen) {
    Value *cond = this->Condition->Codegen(codegen);
    if (cond == nullptr)
        return Helpers::Error(this->Condition->Pos, "Could not evaluate 'if' condition.");
    if (!cond->getType()->isIntegerTy()) // all booleans are integers, so check if the condition is not one and error out.
        return Helpers::Error(this->Condition->Pos, "'if' condition not boolean type.");
    
    Value *booltrue = Helpers::GetBoolean(codegen, true);
    Value *ifCondition = codegen->Builder.CreateICmpUGE(cond, booltrue, "cond");

    Function *func = codegen->Builder.GetInsertBlock()->getParent();
    
    // Create blocks for the 'if' cases.
    BasicBlock *ifthenBB = BasicBlock::Create(codegen->Context, "cond_true", func);
    BasicBlock *ifelseBB = BasicBlock::Create(codegen->Context, "cond_false");
    BasicBlock *ifendBB = BasicBlock::Create(codegen->Context, "cond_merge"); // both blocks merge back here.

    // Create the conditional branch.
    codegen->Builder.CreateCondBr(ifCondition, ifthenBB, ifelseBB);

    // emit 'if.then' block.
    codegen->Builder.SetInsertPoint(ifthenBB);
    bool ifthenStopped;
    std::vector<Value*> ifThenVals = Helpers::EmitBlock(codegen, this->IfBody, true, &ifthenStopped);
    if (!ifthenStopped) { // if there was no return statement within the else block
        codegen->Builder.CreateBr(ifendBB); // go to merge since there was no return in the then block
    }
    ifthenBB = codegen->Builder.GetInsertBlock();

    // emit 'if.else' block
    func->getBasicBlockList().push_back(ifelseBB);
    codegen->Builder.SetInsertPoint(ifelseBB);

    bool ifelseStopped;
    std::vector<Value*> ifElseVals = Helpers::EmitBlock(codegen, this->ElseBody, true, &ifelseStopped);
    if (!ifelseStopped) { // if there was no return statement within the else block
        codegen->Builder.CreateBr(ifendBB); // go to merge since there was no return in the else block
    }
    ifelseBB = codegen->Builder.GetInsertBlock();


    // emit 'if.end' block
    if (!ifelseStopped) {
        func->getBasicBlockList().push_back(ifendBB);
        codegen->Builder.SetInsertPoint(ifendBB);
    }
    return nullptr;
}
Value *AstWhileExpression::Codegen(CodeGenerator *codegen) {
    return nullptr;
}
Value *AstCallExpression::Codegen(CodeGenerator *codegen) {
    // Lookup the name in the global module table.
    Function *CalleeF = codegen->TheModule->getFunction(this->Name);
    if (CalleeF == nullptr)
        return Helpers::Error(this->Pos, "Unknown function, '%s', referenced.", this->Name.c_str());

    // If argument count is mismatched
    if (CalleeF->arg_size() != Args.size())
        return Helpers::Error(this->Pos, "Incorrect number of arguments passed to function '%s'", this->Name.c_str());
    std::vector<Value*> argsvals;
    auto calleeArg = CalleeF->arg_begin();
    for (unsigned i = 0, len = this->Args.size(); i < len; ++i, ++calleeArg){
        Value *val = this->Args[i]->Codegen(codegen);
        bool castSuccess;
        val = Helpers::CreateCastTo(codegen, val, calleeArg->getType(), &castSuccess);
        if (val == nullptr)
            return Helpers::Error(this->Args[i]->Pos, "Argument not valid type, failed to cast to destination.");
        if (castSuccess) // warn that we automatically casted and that there might be a loss of data.
            Helpers::Warning(this->Args[i]->Pos, "Casting from %s to %s, possible loss of data.",
                Helpers::GetLLVMTypeName(val->getType()).c_str(), Helpers::GetLLVMTypeName(calleeArg->getType()).c_str());
        argsvals.push_back(val);
        if (argsvals.back() == nullptr) return nullptr;
    }
    return codegen->Builder.CreateCall(CalleeF, argsvals, "calltmp");
}
Value *AstVariableNode::Codegen(CodeGenerator *codegen) {
    Value *v = codegen->NamedValues[this->Name];
    if (v == nullptr) return Helpers::Error(this->Pos, "Unknown variable name.");
    return codegen->Builder.CreateLoad(v, this->Name.c_str());
}
Value *AstVarNode::Codegen(CodeGenerator *codegen) {
    return nullptr;
}
Function *PrototypeAst::Codegen(CodeGenerator *codegen) {
    // TODO: Serialize the prototype to allow for function overriding.
    std::vector<Type *> argTypes;
    for (auto itr = this->Args.begin(); itr != this->Args.end(); ++itr) {
        Type *type = (*itr).second->GetLLVMType(codegen);
        argTypes.push_back(type);
    }
    FunctionType *funcType = FunctionType::get(this->ReturnType->GetLLVMType(codegen), argTypes, false);
    Function *func = Function::Create(funcType, Function::ExternalLinkage, this->Name, codegen->TheModule);

    // If 'func' conflicted, ther was already something named 'Name'. If it has a body,
    // don't allow redefinition.
    if (func->getName() != this->Name) {
        // Delete the one we just made and get the existing one.
        func->eraseFromParent();
        func = codegen->TheModule->getFunction(this->Name);

        // if func already has a body, reject this.
        if (!func->empty()) {
            return Helpers::Error(this->Pos, "Redefinition of function '%s'", this->Name.c_str());
        }

        // if func took a different number of args, reject
        if (func->arg_size() != this->Args.size()) {
            return Helpers::Error(this->Pos, "Redefinitionof function '%s' with a different number of arguments.", this->Name.c_str());
        }
    }
    return func;
}
// CreateArgumentAllocas - Create an alloca for each argument and register the
// argument in the symbol table so that references to it will succeed.
void PrototypeAst::CreateArgumentAllocas(CodeGenerator *codegen, Function *func) {
    auto arg_itr = func->arg_begin();
    auto this_arg_itr = this->Args.begin();
    for (; this_arg_itr != this->Args.end(); ++this_arg_itr, ++arg_itr) {
        
        std::string argName = this_arg_itr->first;
        AstTypeNode *arg = this_arg_itr->second;

        // Create an alloca for this variable
        AllocaInst *Alloca = Helpers::CreateEntryBlockAlloca(func, argName, arg->GetLLVMType(codegen));

        // Store the initial value
        codegen->Builder.CreateStore(arg_itr, Alloca);
        
        // Add arguments to variable symbol table.
        codegen->NamedValues[argName] = Alloca;
    }
}

Function *FunctionAst::Codegen(CodeGenerator *codegen) {
    codegen->NamedValues.clear();
    Function *func = this->Prototype->Codegen(codegen);
    if (func == nullptr) return nullptr;

    // Create our entry block
    BasicBlock *entryBB = BasicBlock::Create(codegen->Context, "entry", func);
    BasicBlock *retBB = BasicBlock::Create(codegen->Context, "return");
    codegen->ReturnBlock = retBB; // save our return block to jump to.
    codegen->Builder.SetInsertPoint(entryBB);
    this->Prototype->CreateArgumentAllocas(codegen, func);
    // TODO: Check if return type is void and do not create return value, but return void.
    AllocaInst *retVal = Helpers::CreateEntryBlockAlloca(func, COMPILER_RETURN_VALUE_STRING, this->Prototype->ReturnType->GetLLVMType(codegen));
    codegen->NamedValues[COMPILER_RETURN_VALUE_STRING] = retVal;

    auto block = Helpers::EmitBlock(codegen, this->FunctionBody);
    Value *lastVal = nullptr;
    for (auto itr = block.begin(); itr != block.end(); ++itr) {
         lastVal = *itr;
    }

    func->getBasicBlockList().push_back(retBB); // create the return block
    codegen->Builder.SetInsertPoint(retBB); 
    Value *derefRetVal = codegen->Builder.CreateLoad(retVal, COMPILER_RETURN_VALUE_STRING);
    codegen->Builder.CreateRet(derefRetVal);
    retBB = codegen->Builder.GetInsertBlock();

    if (verifyFunction(*func, &errs())) {
        func->dump();
        func->eraseFromParent();
        return Helpers::Error(this->Pos, "Error creating function body.");
    }
    else {
        codegen->TheFPM->run(*func);
    }
    return func;
}


