#include "PrototypeAst.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"

using namespace llvm;

PrototypeAst::PrototypeAst(const std::string &name, AstTypeNode *returnType, const std::vector<std::pair<std::string, AstTypeNode*>> &args,
    bool isVarArgs, int line, int column)
    : Name(name)
    , ReturnType(returnType)
    , Args(args)
    , IsVarArgs(isVarArgs) {
    Pos.LineNumber = line;
    Pos.ColumnNumber = column;
}
PrototypeAst::~PrototypeAst() {
    delete ReturnType;
    for (auto begin = Args.begin(); begin != Args.end(); ++begin) {
        delete begin->second;
    }
}

PossiblePosition PrototypeAst::getPos() const {
    return Pos; 
}
const std::string &PrototypeAst::getName() const {
    return Name; 
}
AstTypeNode *PrototypeAst::getReturnType() const {
    return ReturnType; 
}

Function *PrototypeAst::Codegen(CodeGenerator *codegen) {
    // TODO: Serialize the prototype to allow for function overloading.
    std::vector<Type *> argTypes;
    for (auto itr = this->Args.begin(); itr != this->Args.end(); ++itr) {
        Type *type = itr->second->GetLLVMType(codegen);
        if (type->isVoidTy()) // void is not a valid function parameter type.
            return Helpers::Error(itr->second->getPos(), "'void' not valid function parameter type.");
        argTypes.push_back(type);
    }
    FunctionType *funcType = FunctionType::get(this->ReturnType->GetLLVMType(codegen), argTypes, this->IsVarArgs);
    Function *func = Function::Create(funcType, Function::ExternalLinkage, this->Name, codegen->getTheModule());

    // If 'func' conflicted, ther was already something named 'Name'. If it has a body,
    // don't allow redefinition.
    if (func->getName() != this->Name) {
        // Delete the one we just made and get the existing one.
        func->eraseFromParent();
        func = codegen->getTheModule()->getFunction(this->Name);

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
        AllocaInst *Alloca = Helpers::CreateEntryBlockAlloca(codegen, func, argName, arg->GetLLVMType(codegen));

        // Store the initial value
        codegen->getBuilder().CreateStore(arg_itr, Alloca);

        // Add arguments to variable symbol table.
        codegen->setNamedValue(argName, Alloca);
    }
}
