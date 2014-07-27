#include "llvm/Analysis/Passes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
//#include "llvm/IR/DataLayout.h"
//#include "llvm/IR/DerivedTypes.h"
//#include "llvm/IR/IRBuilder.h"
//#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"c

#include "CodeGenerator.h"
#include "CodeGeneratorHelpers.h"
#include "TreeContainer.h"

#include "DEFINES.h"

using namespace llvm;
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
#ifdef _WIN32
        system("PAUSE");
#endif
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
}

void CodeGenerator::DumpMainModule() {
    auto moduleName = TheModule->getModuleIdentifier().c_str();
    fprintf(stderr, "\n\n============ BEGIN DUMPING MODULE '%s' ============\n\n", moduleName);
    TheModule->dump();
    fprintf(stderr, "\n\n============ END DUMPING MODULE '%s' ============\n\n", moduleName);
}

void CodeGenerator::RunMain() {
    Function *mainFunc = TheModule->getFunction("main");
    if (mainFunc == nullptr) {
        Helpers::Error(PossiblePosition{ -1, -1 }, "No main function found!");
        return;
    }
    void *mainFnPtr = TheExecutionEngine->getPointerToFunction(mainFunc);

    int(*FP)() = (int(*)())mainFnPtr;

    int val = FP();

    printf("%f\n", (double)val);
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
Value *AstBinaryOperatorExpr::VariableAssignment(CodeGenerator *codegen) {
    AstVariableNode *lhse = dynamic_cast<AstVariableNode*>(this->LHS);
    if (lhse == nullptr) // left side of assign operator isn't a variable.
        return Helpers::Error(this->LHS->getPos(), "Destination of assignment operator must be variable.");

    // Codegen to evaluate the expression to store within the variable.
    Value *val = this->RHS->Codegen(codegen);
    if (val == nullptr) return nullptr;

    // Lookup the name of the variable
    Value *variable = codegen->NamedValues[lhse->getName()];
    if (variable == nullptr) return Helpers::Error(this->getPos(), "Unknown variable name '%s'", lhse->getName().c_str());
    
    // Check if the we should cast to
    Type *varType = variable->getType()->getContainedType(0);
    auto valtypeId = val->getType()->getTypeID();
    auto vartypeId = varType->getTypeID();

    val = Helpers::CreateCastTo(codegen, val, varType);
    codegen->Builder.CreateStore(val, variable);
    return val;
}
Value *AstBinaryOperatorExpr::Codegen(CodeGenerator *codegen) {
    if (this->Operator == '=') {
        return this->VariableAssignment(codegen);
    }
    Value *l = this->LHS->Codegen(codegen);
    Value *r = this->RHS->Codegen(codegen);
    if (l == nullptr || r == nullptr) return nullptr;
    Type *lType = l->getType();
    Type *rType = r->getType();
    auto funcPtr = Helpers::GetBinopCodeGenFuncPointer(this->Operator, lType, rType);
    if (funcPtr == nullptr) {
        return Helpers::Error(this->getPos(), "Operator '%s' does not exist for '%s' and '%s'",
            this->OperatorString.c_str(), Helpers::GetLLVMTypeName(lType).c_str(), Helpers::GetLLVMTypeName(rType).c_str());
    }
    return funcPtr(codegen, l, r);
}
Value *AstReturnNode::Codegen(CodeGenerator *codegen) { 
    // TODO: returning void.
    Value *val = this->Expr->Codegen(codegen);
    if (val == nullptr)
        return Helpers::Error(this->getPos(), "Could not evaluate return statement.");
    Type *valType = val->getType();
    Type *retType = codegen->Builder.getCurrentFunctionReturnType();
    if (valType->getTypeID() != retType->getTypeID()) {
        bool castSuccess;
        val = Helpers::CreateCastTo(codegen, val, retType, &castSuccess);
        if (val == nullptr) 
            return Helpers::Error(this->Expr->getPos(), "Could not cast return statement to function return type.");
        if (castSuccess) // warn that we automatically casted and that there might be a loss of data.
            Helpers::Warning(this->Expr->getPos(), "Casting from %s to %s, possible loss of data.",
                Helpers::GetLLVMTypeName(valType).c_str(), Helpers::GetLLVMTypeName(retType).c_str());
    }
    AllocaInst *retVal = codegen->NamedValues[COMPILER_RETURN_VALUE_STRING];
    codegen->Builder.CreateStore(val, retVal);

    Value *derefRetVal = codegen->Builder.CreateLoad(retVal, "retval");
    codegen->Builder.CreateRet(derefRetVal);
    //codegen->Builder.CreateBr(codegen->ReturnBlock);
    return derefRetVal;
}
Value *AstIfElseExpression::Codegen(CodeGenerator *codegen) {
    Value *cond = this->Condition->Codegen(codegen);
    if (cond == nullptr)
        return Helpers::Error(this->Condition->getPos(), "Could not evaluate 'if' condition.");
    if (!cond->getType()->isIntegerTy()) // all booleans are integers, so check if the condition is not one and error out.
        return Helpers::Error(this->Condition->getPos(), "'if' condition not boolean type.");
    Value *booltrue = codegen->Builder.getTrue();
    Value *ifCondition = codegen->Builder.CreateICmpUGE(cond, booltrue, "cond");

    Function *func = codegen->Builder.GetInsertBlock()->getParent();
    
    // Create blocks for the 'if' cases.
    BasicBlock *ifendBB = BasicBlock::Create(codegen->Context, "cond_merge", func, codegen->ReturnBlock); // both blocks merge back here.
    BasicBlock *ifelseBB = BasicBlock::Create(codegen->Context, "cond_false", func, ifendBB);
    BasicBlock *ifthenBB = BasicBlock::Create(codegen->Context, "cond_true", func, ifelseBB);

    // Create the conditional branch.
    codegen->Builder.CreateCondBr(ifCondition, ifthenBB, ifelseBB);
    bool goesToMerge = false;

    // emit 'if.then' block.
    codegen->Builder.SetInsertPoint(ifthenBB);
    bool ifthenStopped;
    std::vector<Value*> ifThenVals = Helpers::EmitBlock(codegen, this->IfBody, true, &ifthenStopped);
    if (ifthenBB->getTerminator() == nullptr) { // if there was no return statement within the else block
        codegen->Builder.CreateBr(ifendBB); // go to merge since there was no return in the then block
        goesToMerge = true;
    }
    ifthenBB = codegen->Builder.GetInsertBlock();

    // emit 'if.else' block
    //func->getBasicBlockList().push_back(ifelseBB);
    codegen->Builder.SetInsertPoint(ifelseBB);

    bool ifelseStopped;
    std::vector<Value*> ifElseVals = Helpers::EmitBlock(codegen, this->ElseBody, true, &ifelseStopped);
    if (ifelseBB->getTerminator() == nullptr) { // if there was no return statement within the else block
        codegen->Builder.CreateBr(ifendBB); // go to merge since there was no return in the else block
        goesToMerge = true;
    }
    ifelseBB = codegen->Builder.GetInsertBlock();

    if (goesToMerge) {
        codegen->Builder.SetInsertPoint(ifendBB);
    }
    else {
        ifendBB->removeFromParent();
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
        return Helpers::Error(this->getPos(), "Unknown function, '%s', referenced.", this->Name.c_str());

    // If argument count is mismatched
    if (CalleeF->arg_size() != Args.size())
        return Helpers::Error(this->getPos(), "Incorrect number of arguments passed to function '%s'", this->Name.c_str());
    std::vector<Value*> argsvals;
    auto calleeArg = CalleeF->arg_begin();
    for (unsigned i = 0, len = this->Args.size(); i < len; ++i, ++calleeArg){
        Value *val = this->Args[i]->Codegen(codegen);
        bool castSuccess;
        val = Helpers::CreateCastTo(codegen, val, calleeArg->getType(), &castSuccess);
        if (val == nullptr)
            return Helpers::Error(this->Args[i]->getPos(), "Argument not valid type, failed to cast to destination.");
        if (castSuccess) // warn that we automatically casted and that there might be a loss of data.
            Helpers::Warning(this->Args[i]->getPos(), "Casting from %s to %s, possible loss of data.",
                Helpers::GetLLVMTypeName(val->getType()).c_str(), Helpers::GetLLVMTypeName(calleeArg->getType()).c_str());
        argsvals.push_back(val);
        if (argsvals.back() == nullptr) return nullptr;
    }
    return codegen->Builder.CreateCall(CalleeF, argsvals, "calltmp");
}
Value *AstVariableNode::Codegen(CodeGenerator *codegen) {
    Value *v = codegen->NamedValues[this->Name];
    if (v == nullptr) return Helpers::Error(this->getPos(), "Unknown variable name.");
    return codegen->Builder.CreateLoad(v, this->Name.c_str());
}
Value *AstVarNode::Codegen(CodeGenerator *codegen) {
    std::vector<AllocaInst*> oldBindings;
    Function *func = codegen->Builder.GetInsertBlock()->getParent();

    IExpressionAST *expr = this->getAssignmentExpression();
    Value *initialVal;
    if (expr == nullptr) { // variable declaration without an assignment, e.g: 'var x : int;'
        initialVal = Helpers::GetDefaultValue(codegen, this->InferredType);
        Helpers::CreateEntryBlockAlloca(func, this->Name.c_str(), this->InferredType->GetLLVMType(codegen));
    }
    else {
        initialVal = expr->Codegen(codegen);
    }

    AllocaInst *Alloca = Helpers::CreateEntryBlockAlloca(func, this->Name, initialVal->getType());
    codegen->Builder.CreateStore(initialVal, Alloca);
    codegen->NamedValues[this->Name] = Alloca;
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
    BasicBlock *retBB = BasicBlock::Create(codegen->Context, "return", func);
    codegen->ReturnBlock = retBB; // save our return block to jump to.
    codegen->Builder.SetInsertPoint(entryBB);
    this->Prototype->CreateArgumentAllocas(codegen, func);
    Type *returnType = this->Prototype->getReturnType()->GetLLVMType(codegen);
    // TODO: Check if return type is void and do not create return value, but return void.
    AllocaInst *retVal = Helpers::CreateEntryBlockAlloca(func, COMPILER_RETURN_VALUE_STRING, returnType);
    codegen->NamedValues[COMPILER_RETURN_VALUE_STRING] = retVal;
    
    Helpers::EmitBlock(codegen, this->FunctionBody, true);
    
    if (codegen->Builder.GetInsertBlock()->getTerminator() == nullptr) { // missing a terminator, try to branch to default return block.
        codegen->Builder.CreateBr(retBB);
    }

    // Switch to the return block
    codegen->Builder.SetInsertPoint(retBB); 
    // Load the return value and get ready to return it.
    Value *derefRetVal = codegen->Builder.CreateLoad(retVal, COMPILER_RETURN_VALUE_STRING);
    // return the function's return value.
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


