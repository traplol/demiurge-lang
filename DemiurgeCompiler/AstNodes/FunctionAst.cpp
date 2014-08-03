#include "llvm/IR/Function.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PassManager.h"

#include "FunctionAst.h"
#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"
#include "../DEFINES.h"

using namespace llvm;

FunctionAst::FunctionAst(PrototypeAst *prototype, const std::vector<IAstExpression*> &functionBody, int line, int column)
    : Prototype(prototype)
    , FunctionBody(functionBody) {
    Pos.LineNumber = line;
    Pos.ColumnNumber = column;
}
FunctionAst::~FunctionAst() {
    delete Prototype;
    while (!FunctionBody.empty()) delete FunctionBody.back(), FunctionBody.pop_back();
}
PossiblePosition FunctionAst::getPos() const { 
    return Pos; 
}
PrototypeAst *FunctionAst::getPrototype() const { 
    return Prototype; 
}

Function *FunctionAst::Codegen(CodeGenerator *codegen) {
    codegen->clearNamedValues();
    Function *func = this->Prototype->Codegen(codegen);
    if (func == nullptr) return Helpers::Error(this->Prototype->getPos(), "Failed to create function!");

    // Create our entry block
    BasicBlock *entryBB = BasicBlock::Create(codegen->getContext(), "entry", func); // head of the function
    BasicBlock *mergeBB = BasicBlock::Create(codegen->getContext(), "merge", func); // where everything merges
    BasicBlock *retBB = BasicBlock::Create(codegen->getContext(), "return", func); // where a return will be branched to on void return type.
    codegen->setReturnBlock(retBB); // save our return block to jump to.
    codegen->getBuilder().SetInsertPoint(entryBB);
    this->Prototype->CreateArgumentAllocas(codegen, func);
    Type *returnType = this->Prototype->getReturnType()->GetLLVMType(codegen);
    AllocaInst *retVal = nullptr;
    if (!func->getReturnType()->isVoidTy()) { // if the function is not void, create a pointer to the return value
        retVal = Helpers::CreateEntryBlockAlloca(codegen, func, COMPILER_RETURN_VALUE_STRING, returnType);
        codegen->setNamedValue(COMPILER_RETURN_VALUE_STRING, retVal);
    }
    codegen->setMergeBlock(mergeBB);
    Helpers::EmitBlock(codegen, this->FunctionBody, false);

    if (codegen->getBuilder().GetInsertBlock()->getTerminator() == nullptr) { // missing a terminator, try to branch to default return block.
        codegen->getBuilder().CreateBr(retBB);
        // Switch to the return block
        codegen->getBuilder().SetInsertPoint(retBB);
        // Load the return value and get ready to return it.
        if (func->getReturnType()->isVoidTy()) {
            codegen->getBuilder().CreateRetVoid();
        }
        else if (retVal != nullptr) {
            Value *derefRetVal = codegen->getBuilder().CreateLoad(retVal, COMPILER_RETURN_VALUE_STRING);
            // return the function's return value.
            codegen->getBuilder().CreateRet(derefRetVal);
        } // if something else went wrong, we shouldn't reach this far
        retBB = codegen->getBuilder().GetInsertBlock();
    }
    else {
        retBB->removeFromParent();
    }

    if (mergeBB->getTerminator() == nullptr) { // do some branch cleanup
        mergeBB->removeFromParent();
    }
    if (verifyFunction(*func, &errs())) { // more branch cleanup
        func->dump();
        func->eraseFromParent();
        return Helpers::Error(this->Pos, "Error creating function body.");
    }
    else { // try to optimize the function by running the function pass manager
        //codegen->getTheFPM()->run(*func);
    }
    return func; // might as well return the generated function for potential closure support later.
}
