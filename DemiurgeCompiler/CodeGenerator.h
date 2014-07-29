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
    // Initializes the code generator.
    CodeGenerator();
    ~CodeGenerator();
    // Runs the code generator on the ASTs passed
    bool GenerateCode(TreeContainer *trees);
    
    // Dumps the main module
    void DumpMainModule();

    // Dumps the last module generated
    void DumpLastModule();

    // Caches the last module generated
    void CacheLastModule();

    // Runs the main function in the last module generated.
    void RunMain();

    // Returns the context
    llvm::LLVMContext &getContext() const { return Context; }
    
    // Returns the module 
    llvm::Module *getTheModule() const { return TheModule; }

    // Returns the function pass manager
    llvm::FunctionPassManager *getTheFPM() const { return TheFPM; }

    // Returns the execution engine
    llvm::ExecutionEngine *getTheExecutionEngine() const { return TheExecutionEngine; }

    // Returns the instruction builder
    llvm::IRBuilder<> &getBuilder() { return Builder; }

    // Returns the block to merge to after scope goes away.
    llvm::BasicBlock *getMergeBlock() const { return MergeBlock; }
    // Sets the merge block/
    void setMergeBlock(llvm::BasicBlock *mergeBlock) { this->MergeBlock = mergeBlock; }
    
    // Returns the functions return block
    llvm::BasicBlock *getReturnBlock() const { return ReturnBlock; }
    // Sets the functions return block
    void setReturnBlock(llvm::BasicBlock *returnBlock) { this->ReturnBlock = returnBlock; }
    
    // Clears the named values
    void clearNamedValues() { NamedValues.clear(); }
    // Returns the AllocaInst at a given key
    llvm::AllocaInst *getNamedValue(std::string key) const { return NamedValues.at(key); }
    // Sets the AllocaInst at a given key if it does not exist yet, and returns a <itr, bool> pair
    std::pair<std::map<std::string, llvm::AllocaInst*>::iterator, bool> setNamedValue(std::string key, llvm::AllocaInst *val) { 
        return NamedValues.insert({ key, val }); 
    }

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