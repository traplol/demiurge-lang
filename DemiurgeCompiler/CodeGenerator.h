#ifndef _CODE_GENERATOR_H
#define _CODE_GENERATOR_H

#include <vector>
#include <map>
#include <string>

#include "llvm/IR/IRBuilder.h"

// TODO: Module and module component container.
// TODO: vector of the a module component containers.

struct TreeContainer;
namespace llvm {
    class ExecutionEngine;
    class FunctionPassManager;
}

class CodeGenerator {
public:
    CodeGenerator();
    ~CodeGenerator();
    bool GenerateCode(TreeContainer *trees);
    void DumpMainModule();
    void DumpLastModule();
    void CacheLastModule();
    void RunMain();

    llvm::LLVMContext &getContext() const { return Context; }
    
    llvm::Module *getTheModule() const { return TheModule; }
    llvm::FunctionPassManager *getTheFPM() const { return TheFPM; }
    llvm::ExecutionEngine *getTheExecutionEngine() const { return TheExecutionEngine; }

    llvm::IRBuilder<> &getBuilder() { return Builder; }
    std::map<std::string, llvm::AllocaInst*> &getNamedValues() { return NamedValues; }

    llvm::BasicBlock *getMergeBlock() const { return MergeBlock; }
    void setMergeBlock(llvm::BasicBlock *mergeBlock) { this->MergeBlock = mergeBlock; }
    llvm::BasicBlock *getReturnBlock() const { return ReturnBlock; }
    void setReturnBlock(llvm::BasicBlock *returnBlock) { this->ReturnBlock = returnBlock; }
    
private:
    llvm::LLVMContext &Context;
    llvm::IRBuilder<> Builder;
    llvm::Module *TheModule;
    llvm::FunctionPassManager *TheFPM;
    llvm::ExecutionEngine *TheExecutionEngine;
    llvm::BasicBlock *MergeBlock;
    llvm::BasicBlock *ReturnBlock;
    std::map<std::string, llvm::AllocaInst*> NamedValues;
    void InitJitOutputFunctions();
};

#endif