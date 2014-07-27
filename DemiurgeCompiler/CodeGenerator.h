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

    llvm::LLVMContext &Context;
    llvm::IRBuilder<> Builder;
    llvm::Module *TheModule;
    llvm::FunctionPassManager *TheFPM;
    llvm::ExecutionEngine *TheExecutionEngine;
    llvm::BasicBlock *MergeBlock;
    llvm::BasicBlock *ReturnBlock;
    std::map<std::string, llvm::AllocaInst*> NamedValues;
private:
    void InitJitOutputFunctions();
};

#endif