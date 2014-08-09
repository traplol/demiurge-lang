#ifndef _CODE_GENERATOR_H
#define _CODE_GENERATOR_H

#include <vector>
#include <map>
#include <string>

#include "llvm/IR/IRBuilder.h"

// TODO: Module and module component container.
// TODO: vector of the a module component containers.

struct TreeContainer;
class FunctionAst;
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
    llvm::LLVMContext &getContext() const;
    
    // Returns the module 
    llvm::Module *getTheModule() const;

    // Returns the function pass manager
    llvm::FunctionPassManager *getTheFPM() const;

    // Returns the execution engine
    llvm::ExecutionEngine *getTheExecutionEngine() const;

    // Returns the instruction builder
    llvm::IRBuilder<> &getBuilder();

    // Returns the block to merge to after scope goes away.
    llvm::BasicBlock *getOutsideBlock() const;
    // Sets the merge block/
    void setOutsideBlock(llvm::BasicBlock *mergeBlock);
    
    // Returns the functions return block
    llvm::BasicBlock *getReturnBlock() const;
    
    // Sets the functions return block
    void setReturnBlock(llvm::BasicBlock *returnBlock);
    
    // Clears the named values
    void clearNamedValues();
    
    // Returns the AllocaInst at a given key
    llvm::AllocaInst *getNamedValue(const std::string &key) const;
    
    // Pushes the key to the Scope Stack and sets the AllocaInst at a given key 
    // if it does not exist yet, and returns a <itr, bool> pair
    std::pair<std::map<std::string, llvm::AllocaInst*>::iterator, bool> setNamedValue(std::string key, llvm::AllocaInst *val);
    // Erases an AllocaInst from the map.
    void eraseNamedValue(const std::string &key);
    
    // Clears the Scope Stack for the local scope and removes them from the NamedValues set.
    void popFromScopeStack(unsigned howMany);

    // Increment the variable count
    void incrementVarCount();
    // Increment the nest/scope depth
    void incrementNestDepth();
    // Decrement the nest/scope depth
    void decrementNestDepth();
    // Returns the nest/scope depth
    unsigned getNestDepth() const;
    // Returns the variable count.
    unsigned getVarCount() const;
    // Returns the function currently being created.
    llvm::Function *getCurrentFunction() const;
    void setCurrentFunction(llvm::Function* func);
private:
    llvm::LLVMContext &Context;
    llvm::IRBuilder<> Builder;
    llvm::Module *TheModule;
    llvm::FunctionPassManager *TheFPM;
    llvm::ExecutionEngine *TheExecutionEngine;
    llvm::BasicBlock *OutsideBlock;
    llvm::BasicBlock *ReturnBlock;
    std::map<std::string, llvm::AllocaInst*> NamedValues;
    void initJitOutputFunctions();
    bool declareFunctions(TreeContainer *trees);
    std::vector<std::string> ScopeStack;
    unsigned VarCount = 0;
    unsigned NestDepth = 0;
    llvm::Function *CurrentFunction;
};

#endif