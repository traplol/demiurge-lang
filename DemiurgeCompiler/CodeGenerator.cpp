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

#include <sstream>

#include "CodeGenerator.h"
#include "ASTNodes.h"
#include "TreeContainer.h"

using namespace llvm;

namespace Helpers {
    static nullptr_t Error(const char *fmt, ...);
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
        Helpers::Error("No main function found!");
        return;
    }
    void *mainFnPtr = TheExecutionEngine->getPointerToFunction(mainFunc);
    
    double(*FP)() = (double(*)())mainFnPtr;
    
    double val = FP();

    printf("%f\n",(double)val);
}
namespace Helpers {
    static nullptr_t Error(const char *fmt, ...) {
        char buf[4096];
        va_list args;
        va_start(args, fmt);
        vsnprintf_s(buf, 4096, fmt, args);
        va_end(args);
        fprintf(stderr, "Error: %s\n", buf);
        return nullptr;
    }

    static void Warning(const char *fmt, ...) {
        char buf[4096];
        va_list args;
        va_start(args, fmt);
        vsnprintf_s(buf, 4096, fmt, args);
        va_end(args);
        fprintf(stderr, "Warning: %s\n", buf);
    }
    
    // PhiNodeAddIncoming - calls PHINode::addIncoming with all of the llvm::Value* in the 'vals' std::vector
    void PhiNodeAddIncoming(PHINode *phiNode, BasicBlock *block, const std::vector<Value*> &vals) {
        for (unsigned i = 0, size = vals.size(); i < size; ++i) {
            phiNode->addIncoming(vals[i], block);
        }
    }
    // EmitBlock - Emits a std::vector of ast::IAstExpr* expressions and returns their value
    // in a std::vector<llvm::Value*>
    std::vector<Value*> EmitBlock(CodeGenerator *codegen, const std::vector<IExpressionAST*> &block) {
        std::vector<Value*> vals;
        for (unsigned i = 0, size = block.size(); i < size; ++i) {
            IExpressionAST *expr = block[i];
            Value *val = expr->Codegen(codegen);
            vals.push_back(val);
        }
        return vals;
    }
    // CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
    // the function.  This is used for mutable variables etc.
    AllocaInst* CreateEntryBlockAlloca(Function *function, const std::string &varName, Type *type) {
        IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
        return tmpBuilder.CreateAlloca(type, 0, varName.c_str());
    }

    // Creates a LLVM::Value* with double type from the value passed.
    Value *GetDouble(CodeGenerator *codegen, double val) {
        return ConstantFP::get(codegen->Context, APFloat(val));
    }
    // Creates a LLVM::Value* with integer64 type from the value passed.
    Value *GetInt(CodeGenerator *codegen, unsigned long long int val, int bitwidth) {
        return ConstantInt::get(codegen->Context, APInt(bitwidth, val));
    }
    // Creates a LLVM::Value* with integer1 type from the value passed.
    Value *GetBoolean(CodeGenerator *codegen, bool val) {
        return GetInt(codegen, val, 1);
    }
    // Creates a LLVM::Value* with integer8* type from the value passed.
    Value *GetString(CodeGenerator *codegen, std::string val) {
        return ConstantDataArray::getString(codegen->Context, val.c_str());
    }

    // Returns a string representation of the passed llvm type.
    std::string GetLLVMTypeName(Type *Ty) {
        switch (Ty->getTypeID()) {
        default: return "not supported";
        case Type::TypeID::DoubleTyID: return "double";
        case Type::TypeID::FloatTyID: return "float";
        case Type::TypeID::ArrayTyID: return "array";
        case Type::TypeID::FunctionTyID: return "function";
        case Type::TypeID::IntegerTyID: return "int";
        case Type::TypeID::PointerTyID: return "pointer";
        case Type::TypeID::StructTyID: return "struct";
        case Type::TypeID::VoidTyID: return "void";
        }
    }

    // Will attempt to cast one value to another type and sets 'castSuccessful' to true if a cast happened, otherwise false.
    Value *CreateCastTo(CodeGenerator *codegen, Value *val, Type *castToType, bool *castSuccessful = nullptr) {
        if (castSuccessful != nullptr)
            *castSuccessful = false;
        Type *valType = val->getType();
        if (valType->getTypeID() == castToType->getTypeID())
            return val;
        if (valType->canLosslesslyBitCastTo(castToType)) {
            if (castSuccessful != nullptr)
                *castSuccessful = true;
            return codegen->Builder.CreateBitCast(val, castToType, "bitcasted");
        }
        if (CastInst::isCastable(valType, castToType)) {
            if (castSuccessful != nullptr)
                *castSuccessful = true;
            return codegen->Builder.CreateCast(CastInst::getCastOpcode(val, true, castToType, true), val, castToType, "castto");
        }
        return nullptr;
    }

    namespace LLVMBinOperations{
        typedef Value*(*BinOpCodeGenFuncPtr)(CodeGenerator*, Value*, Value*);
        typedef BinOpCodeGenFuncPtr(*GetFuncPtr)(TokenType);
        typedef std::map<Type::TypeID, std::map<Type::TypeID, GetFuncPtr > > LookupTableMapMap;

        Value *FailedLookupFunc(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return Error("Failed to lookup binary operator function.");
        }

        // int:int

        Value *intintAdd(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateAdd(lhs, rhs, "intintAdd");
        }
        Value *intintSub(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateSub(lhs, rhs, "intintSub");
        }
        Value *intintMul(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateMul(lhs, rhs, "intintMul");
        }
        Value *intintDiv(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateSDiv(lhs, rhs, "intintDiv");
        }
        Value *intintMod(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateSRem(lhs, rhs, "intintMod");
        }
        Value *intintLT(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateICmpSLT(lhs, rhs, "intintLT");
        }
        Value *intintGT(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateICmpSGT(lhs, rhs, "intintGT");
        }
        Value *intintLE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateICmpSLE(lhs, rhs, "intintLE");
        }
        Value *intintGE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateICmpSGE(lhs, rhs, "intintGE");
        }
        Value *intintEQ(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateICmpEQ(lhs, rhs, "intintEQ");
        }
        Value *intintNE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateICmpNE(lhs, rhs, "intintNE");
        }

        BinOpCodeGenFuncPtr getIntIntFuncPtr(TokenType type) {
            switch (type) {
            default: return FailedLookupFunc;
            case '+': return intintAdd;
            case '-': return intintSub;
            case '*': return intintMul;
            case '/': return intintDiv;
            case '%': return intintMod;
            case '<': return intintLT;
            case '>': return intintGT;
            case tok_lessequal: return intintLE;
            case tok_greatequal: return intintGE;
            case tok_equalequal: return intintEQ;
            case tok_notequal: return intintNE;
            }
        }

        // int:double | double:int
        Value *intToDouble(CodeGenerator *codegen, Value *intValue) {
            return codegen->Builder.CreateSIToFP(intValue, Type::getDoubleTy(codegen->Context), "uintToFp");
        }
        Value *doubleToInt(CodeGenerator *codegen, Value *doubleValue) {
            return codegen->Builder.CreateSIToFP(doubleValue, Type::getInt64Ty(codegen->Context), "fpToUint");
        }

        Value *intdoubleAdd(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy())
                lhs = intToDouble(codegen, lhs);
            else rhs = intToDouble(codegen, rhs);

            return codegen->Builder.CreateFAdd(lhs, rhs, "intdoubleAdd");
        }
        Value *intdoubleSub(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy())
                lhs = intToDouble(codegen, lhs);
            else rhs = intToDouble(codegen, rhs);

            return codegen->Builder.CreateFSub(lhs, rhs, "intdoubleSub");
        }
        Value *intdoubleMul(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy())
                lhs = intToDouble(codegen, lhs);
            else rhs = intToDouble(codegen, rhs);

            return codegen->Builder.CreateFMul(lhs, rhs, "intdoubleMul");
        }
        Value *intdoubleDiv(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy())
                lhs = intToDouble(codegen, lhs);
            else rhs = intToDouble(codegen, rhs);

            return codegen->Builder.CreateFDiv(lhs, rhs, "intdoubleDiv");
        }
        Value *intdoubleMod(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy())
                lhs = intToDouble(codegen, lhs);
            else rhs = intToDouble(codegen, rhs);

            return codegen->Builder.CreateFRem(lhs, rhs, "intdoubleMod");
        }
        Value *intdoubleLT(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy())
                lhs = intToDouble(codegen, lhs);
            else rhs = intToDouble(codegen, rhs);

            return codegen->Builder.CreateFCmpULT(lhs, rhs, "intdoubleLT");
        }
        Value *intdoubleGT(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy())
                lhs = intToDouble(codegen, lhs);
            else rhs = intToDouble(codegen, rhs);

            return codegen->Builder.CreateFCmpUGT(lhs, rhs, "intdoubleGT");
        }
        Value *intdoubleLE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy())
                lhs = intToDouble(codegen, lhs);
            else rhs = intToDouble(codegen, rhs);

            return codegen->Builder.CreateFCmpULE(lhs, rhs, "intdoubleLE");
        }
        Value *intdoubleGE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy())
                lhs = intToDouble(codegen, lhs);
            else rhs = intToDouble(codegen, rhs);

            return codegen->Builder.CreateFCmpUGE(lhs, rhs, "intdoubleGE");
        }
        Value *intdoubleEQ(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy())
                lhs = intToDouble(codegen, lhs);
            else rhs = intToDouble(codegen, rhs);

            return codegen->Builder.CreateFCmpOEQ(lhs, rhs, "intdoubleEQ");
        }
        Value *intdoubleNE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->getTypeID() == Type::TypeID::IntegerTyID)
                lhs = intToDouble(codegen, lhs);
            else rhs = intToDouble(codegen, rhs);

            return codegen->Builder.CreateFCmpONE(lhs, rhs, "intdoubleNE");
        }

        BinOpCodeGenFuncPtr getIntDoubleFuncPtr(TokenType type) {
            switch (type) {
            default: return FailedLookupFunc;
            case '+': return intdoubleAdd;
            case '-': return intdoubleSub;
            case '*': return intdoubleMul;
            case '/': return intdoubleDiv;
            case '%': return intdoubleMod;
            case '<': return intdoubleLT;
            case '>': return intdoubleGT;
            case tok_lessequal: return intdoubleLE;
            case tok_greatequal: return intdoubleGE;
            case tok_equalequal: return intdoubleEQ;
            case tok_notequal: return intdoubleNE;
            }
        }

        // double:double
        Value *doubledoubleAdd(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateFAdd(lhs, rhs, "doubledoubleAdd");
        }
        Value *doubledoubleSub(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateFSub(lhs, rhs, "doubledoubleSub");
        }
        Value *doubledoubleMul(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateFMul(lhs, rhs, "doubledoubleMul");
        }
        Value *doubledoubleDiv(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateFDiv(lhs, rhs, "doubledoubleDiv");
        }
        Value *doubledoubleMod(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateFRem(lhs, rhs, "doubledoubleMod");
        }
        Value *doubledoubleLT(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateFCmpULT(lhs, rhs, "doubledoubleLT");
        }
        Value *doubledoubleGT(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateFCmpUGT(lhs, rhs, "doubledoubleGT");
        }
        Value *doubledoubleLE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateFCmpULE(lhs, rhs, "doubledoubleLE");
        }
        Value *doubledoubleGE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateFCmpUGE(lhs, rhs, "doubledoubleGE");
        }
        Value *doubledoubleEQ(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateFCmpOEQ(lhs, rhs, "doubledoubleEQ");
        }
        Value *doubledoubleNE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->Builder.CreateFCmpONE(lhs, rhs, "doubledoubleNE");
        }

        BinOpCodeGenFuncPtr getDoubleDoubleFuncPtr(TokenType type) {
            switch (type) {
            default: return FailedLookupFunc;
            case '+': return doubledoubleAdd;
            case '-': return doubledoubleSub;
            case '*': return doubledoubleMul;
            case '/': return doubledoubleDiv;
            case '%': return doubledoubleMod;
            case '<': return doubledoubleLT;
            case '>': return doubledoubleGT;
            case tok_lessequal: return doubledoubleLE;
            case tok_greatequal: return doubledoubleGE;
            case tok_equalequal: return doubledoubleEQ;
            case tok_notequal: return doubledoubleNE;
            }
        }

        LookupTableMapMap getBinOpLookupTable() {
            LookupTableMapMap lookupTable;
            lookupTable[Type::IntegerTyID][Type::IntegerTyID] = getIntIntFuncPtr;
            lookupTable[Type::IntegerTyID][Type::DoubleTyID] = getIntDoubleFuncPtr;
            lookupTable[Type::DoubleTyID][Type::IntegerTyID] = getIntDoubleFuncPtr;
            lookupTable[Type::DoubleTyID][Type::DoubleTyID] = getDoubleDoubleFuncPtr;
            return lookupTable;
        }
    }

    // Returns a pointer to a function which will generate the proper LLVM code to handle the operator and types passed.
    LLVMBinOperations::BinOpCodeGenFuncPtr GetBinopCodeGenFuncPointer(TokenType Operator, Type *lType, Type *rType) {
        auto func = LLVMBinOperations::getBinOpLookupTable()[lType->getTypeID()][rType->getTypeID()];
        if (func == nullptr) return nullptr;
        return func(Operator);
    }


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
    if (funcPtr != nullptr)
        funcPtr(codegen, l, r);
}
Value *AstReturnNode::Codegen(CodeGenerator *codegen) {
    Value *val = this->Expr->Codegen(codegen);
    if (val == nullptr)
        return Helpers::Error("(%d:%d) - Could not evaluate return statement.", this->LineNumber, this->ColumnNumber);
    Type *valType = val->getType();
    Type *retType = codegen->Builder.getCurrentFunctionReturnType();
    if (valType->getTypeID() != retType->getTypeID()) {
        bool castSuccess;
        val = Helpers::CreateCastTo(codegen, val, retType, &castSuccess);
        if (val == nullptr) 
            return Helpers::Error("(%d:%d) - Could not cast return statement to function return type.", this->LineNumber, this->ColumnNumber);
        if (castSuccess) // warn that we automatically casted and that there might be a loss of data.
            Helpers::Warning("(%d:%d) - Casting from %s to %s, possible loss of data.", this->LineNumber, this->ColumnNumber, 
                Helpers::GetLLVMTypeName(valType).c_str(), Helpers::GetLLVMTypeName(retType).c_str());
    }
    return codegen->Builder.CreateRet(val);
}
Value *AstIfElseExpression::Codegen(CodeGenerator *codegen) {
    Value *cond = this->Condition->Codegen(codegen);
    auto type = cond->getType()->getTypeID();
    if (cond == nullptr)
        return Helpers::Error("(%d:%d) - Could not evaluate 'if' condition.", this->Condition->LineNumber, this->Condition->ColumnNumber);
    if (!cond->getType()->isIntegerTy()) // all booleans are integers, so check if the condition is not one and error out
        return Helpers::Error("(%d:%d) - 'if' condition not boolean type.", this->Condition->LineNumber, this->Condition->ColumnNumber);

    Value *booltrue = Helpers::GetBoolean(codegen, true);
    codegen->Builder.CreateICmpUGE(cond, booltrue, "ifcondition");



    return nullptr;
}
Value *AstWhileExpression::Codegen(CodeGenerator *codegen) {
    return nullptr;
}
Value *AstCallExpression::Codegen(CodeGenerator *codegen) {
    // Lookup the name in the global module table.
    Function *CalleeF = codegen->TheModule->getFunction(this->Name);
    if (CalleeF == nullptr)
        return Helpers::Error("(%d:%d) - Unknown function, '%s', referenced.",this->LineNumber, this->ColumnNumber, this->Name.c_str());

    // If argument count is mismatched
    if (CalleeF->arg_size() != Args.size())
        return Helpers::Error("(%d:%d) - Incorrect number of arguments passed to function '%s'", this->LineNumber, this->ColumnNumber, this->Name.c_str());
    std::vector<Value*> argsvals;
    auto calleeArg = CalleeF->arg_begin();
    for (unsigned i = 0, len = this->Args.size(); i < len; ++i, ++calleeArg){
        Value *val = this->Args[i]->Codegen(codegen);
        bool castSuccess;
        val = Helpers::CreateCastTo(codegen, val, calleeArg->getType(), &castSuccess);
        if (val == nullptr)
            return Helpers::Error("(%d:%d) - Argument not valid type, failed to cast to destination.", this->LineNumber, this->ColumnNumber);
        if (castSuccess) // warn that we automatically casted and that there might be a loss of data.
            Helpers::Warning("(%d:%d) - Casting from %s to %s, possible loss of data.", this->LineNumber, this->ColumnNumber,
                Helpers::GetLLVMTypeName(val->getType()).c_str(), Helpers::GetLLVMTypeName(calleeArg->getType()).c_str());
        argsvals.push_back(val);
        if (argsvals.back() == nullptr) return nullptr;
    }
    return codegen->Builder.CreateCall(CalleeF, argsvals, "calltmp");
}
Value *AstVariableNode::Codegen(CodeGenerator *codegen) {
    Value *v = codegen->NamedValues[this->Name];
    if (v == nullptr) return Helpers::Error("(%d:%d) - Unknown variable name.", this->LineNumber, this->ColumnNumber);
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
            return Helpers::Error("(%d:%d) - Redefinition of function '%s'", this->LineNumber, this->ColumnNumber, this->Name.c_str());
        }

        // if func took a different number of args, reject
        if (func->arg_size() != this->Args.size()) {
            return Helpers::Error("(%d:%d) - Redefinitionof function '%s' with a different number of arguments.", this->LineNumber, this->ColumnNumber, this->Name.c_str());
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
    BasicBlock *entry = BasicBlock::Create(codegen->Context, "entry", func);
    codegen->Builder.SetInsertPoint(entry);
    this->Prototype->CreateArgumentAllocas(codegen, func);

    auto block = Helpers::EmitBlock(codegen, this->FunctionBody);
    for (auto itr = block.begin(); itr != block.end(); ++itr) {
        Value *val = *itr;
    }

    if (verifyFunction(*func, &errs())) {
        func->eraseFromParent();
        return Helpers::Error("(%d:%d) - Error creating function body.", this->Prototype->LineNumber, this->Prototype->ColumnNumber);
    }
    else {
        codegen->TheFPM->run(*func);
    }
    return func;
}


