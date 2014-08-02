#include "llvm/Analysis/Passes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/IR/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"

#include "CodeGenerator.h"
#include "CodeGeneratorHelpers.h"
#include "../Compiler/TreeContainer.h"
#include "../DEFINES.h"
#include "../DemiurgeJitOutputFunctions.h"

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
    initJitOutputFunctions();
}


CodeGenerator::~CodeGenerator() {
}

void updateGMap(CodeGenerator *codegen, Type *returnType, const char *name, void *addr, Type *argType, bool isVarArgs = false) {
    std::vector<Type*> args(1, argType);
    FunctionType *funcType = FunctionType::get(returnType, args, isVarArgs);
    Function *func = Function::Create(funcType, Function::ExternalLinkage, name, codegen->getTheModule());
    codegen->getTheExecutionEngine()->updateGlobalMapping(func, addr);
}

void CodeGenerator::initJitOutputFunctions() {
    /* 
     * This is obsolete unless you don't want to export the function but still want it 
     * to be accessible within the compiler.
     */
#define VOID_TYPE Type::getVoidTy(this->Context)
#define STRING_TYPE Type::getInt8PtrTy(this->Context)
#define INT_TYPE Type::getInt64Ty(this->Context)
#define DOUBLE_TYPE Type::getDoubleTy(this->Context)

    
    //updateGMap(this, VOID_TYPE, "print", &print, STRING_TYPE);
    //updateGMap(this, VOID_TYPE, "println", &println, STRING_TYPE);
    //updateGMap(this, VOID_TYPE, "printd", &printd, DOUBLE_TYPE);
    //updateGMap(this, VOID_TYPE, "printi", &printi, INT_TYPE);
    //updateGMap(this, VOID_TYPE, "printc", &printc, INT_TYPE);
    //updateGMap(this, VOID_TYPE, "printf", &_printf, STRING_TYPE, true);


#undef VOID_TYPE
#undef STRING_TYPE
#undef INT_TYPE
#undef DOUBLE_TYPE
}

bool CodeGenerator::declareFunctions(TreeContainer *trees) {
    for (int i = 0, e = trees->ExternalDeclarations.size(); i < e; ++i) { // declare external declarations
        if (trees->ExternalDeclarations[i]->Codegen(this) == nullptr) return false;
    }
    for (int i = 0, e = trees->FunctionDefinitions.size(); i < e; ++i) { // declare user functions
        if (trees->FunctionDefinitions[i]->getPrototype()->Codegen(this) == nullptr) return false;
    }
}

bool CodeGenerator::GenerateCode(TreeContainer *trees) {

    if (!declareFunctions(trees)) return false; // declare functions
    
    for (int i = 0, e = trees->FunctionDefinitions.size(); i < e; ++i) { // define the functions
        if (trees->FunctionDefinitions[i]->Codegen(this) == nullptr) return false;
    }

    // TODO: Top level variables and declare/define them before function declarations.
    for (int i = 0, e = trees->TopLevelExpressions.size(); i < e; ++i) {
        if (trees->TopLevelExpressions[i]->Codegen(this) == nullptr) return false;
    }
    return true;
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
    FP();
}

// Returns the context
LLVMContext &CodeGenerator::getContext() const { 
    return Context; 
}

// Returns the module 
Module *CodeGenerator::getTheModule() const { 
    return TheModule; 
}

// Returns the function pass manager
FunctionPassManager *CodeGenerator::getTheFPM() const { 
    return TheFPM; 
}

// Returns the execution engine
ExecutionEngine *CodeGenerator::getTheExecutionEngine() const { 
    return TheExecutionEngine; 
}

// Returns the instruction builder
IRBuilder<> &CodeGenerator::getBuilder() { 
    return Builder; 
}

// Returns the block to merge to after scope goes away.
BasicBlock *CodeGenerator::getMergeBlock() const { 
    return MergeBlock; 
}
// Sets the merge block/
void CodeGenerator::setMergeBlock(BasicBlock *mergeBlock) { 
    this->MergeBlock = mergeBlock; 
}

// Returns the functions return block
BasicBlock *CodeGenerator::getReturnBlock() const { 
    return ReturnBlock; 
}
// Sets the functions return block
void CodeGenerator::setReturnBlock(BasicBlock *returnBlock) { 
    this->ReturnBlock = returnBlock; 
}

// Clears the named values
void CodeGenerator::clearNamedValues() { NamedValues.clear(); }
// Returns the AllocaInst at a given key
AllocaInst *CodeGenerator::getNamedValue(const std::string &key) const {
    if (NamedValues.count(key))
        return NamedValues.at(key); 
    return nullptr;
}
// Pushes the key to the Scope Stack and sets the AllocaInst at a given key 
// if it does not exist yet, and returns a <itr, bool> pair
std::pair<std::map<std::string, AllocaInst*>::iterator, bool> CodeGenerator::setNamedValue(std::string key, AllocaInst *val) {
    auto success = NamedValues.insert({ key, val });
    if (success.second) { // if the variable was successfully created, push it onto the stack.
        ScopeStack.push_back(key);
    }
    return success;
}
// Erases an AllocaInst from the map.
void CodeGenerator::eraseNamedValue(const std::string &key) { 
    NamedValues.erase(key); 
}

// Clears the Scope Stack for the local scope and removes them from the NamedValues set.
void CodeGenerator::popFromScopeStack(unsigned howMany) {
    while (!ScopeStack.empty() && howMany-- && VarCount--) {
        eraseNamedValue(ScopeStack.back());
        ScopeStack.pop_back();
    }
}
void CodeGenerator::incrementVarCount() { 
    VarCount++; 
}

unsigned int CodeGenerator::getVarCount() const { 
    return VarCount; 
}
