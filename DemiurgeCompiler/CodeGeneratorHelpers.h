#ifndef _CODE_GENERATOR_HELPERS_H
#define _CODE_GENERATOR_HELPERS_H
#include <map>
#include <vector>
#include "TokenTypes.h"
#include "PossiblePosition.h"

namespace llvm {
    class Value;
    class Type;
    class Function;
    class AllocaInst;
}
class IExpressionAST;
class CodeGenerator;
class AstTypeNode;

namespace Helpers {

    nullptr_t Error(PossiblePosition pos, const char *fmt, ...);
    void Warning(PossiblePosition pos, const char *fmt, ...);

    namespace BinOperations {
        typedef llvm::Value*(*BinOpCodeGenFuncPtr)(CodeGenerator*, llvm::Value*, llvm::Value*);
        typedef BinOpCodeGenFuncPtr(*GetFuncPtr)(TokenType);


        llvm::Value *FailedLookupFunc(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);

        // int:int

        llvm::Value *intintAdd(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintSub(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintMul(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintDiv(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintMod(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintAnd(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintOr(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintXor(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintSHL(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintSHR(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintLT(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintGT(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintLE(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintGE(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintEQ(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intintNE(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);

        // int:double | double:int
        llvm::Value *intToDouble(CodeGenerator *codegen, llvm::Value *intValue);
        llvm::Value *doubleToInt(CodeGenerator *codegen, llvm::Value *doubleValue);

        llvm::Value *intdoubleAdd(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intdoubleSub(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intdoubleMul(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intdoubleDiv(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intdoubleMod(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intdoubleLT(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intdoubleGT(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intdoubleLE(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intdoubleGE(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intdoubleEQ(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *intdoubleNE(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);

        // double:double
        llvm::Value *doubledoubleAdd(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *doubledoubleSub(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *doubledoubleMul(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *doubledoubleDiv(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *doubledoubleMod(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *doubledoubleLT(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *doubledoubleGT(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *doubledoubleLE(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *doubledoubleGE(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *doubledoubleEQ(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
        llvm::Value *doubledoubleNE(CodeGenerator *codegen, llvm::Value *lhs, llvm::Value *rhs);
    }

    // Returns a pointer to a function which will generate the proper LLVM code to handle the operator and types passed.
    BinOperations::BinOpCodeGenFuncPtr GetBinopCodeGenFuncPointer(TokenType Operator, llvm::Type *lType, llvm::Type *rType);

    // EmitBlock - Emits a std::vector of ast::IAstExpr* expressions and returns their value
    // in a std::vector<llvm::Value*>
    std::vector<llvm::Value*> EmitBlock(CodeGenerator *codegen, const std::vector<IExpressionAST*> &block, bool stopAtFirstReturn = false, bool *stopped = nullptr);
  
    // CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
    // the function.  This is used for mutable variables etc.
    llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function *function, const std::string &varName, llvm::Type *type);
  
    // Creates a LLVM::Value* of double type from the llvm::Value passed.
    llvm::Value *GetDouble(CodeGenerator *codegen, double val);
    
    // Creates a LLVM::Value* of integer type with specified width from the value passed.
    llvm::Value *GetInt(CodeGenerator *codegen, unsigned long long int val, int bitwidth);
    
    // Creates a LLVM::Value* of integer64 type with a value of zero.
    llvm::Value *GetZero_64(CodeGenerator *codegen);
    
    // Creates a LLVM::Value* of integer64 type with a value of one.
    llvm::Value *GetOne_64(CodeGenerator *codegen);
    
    // Creates a LLVM::Value* of integer64 type with a value of two.
    llvm::Value *GetTwo_64(CodeGenerator *codegen);

    // Creates a LLVM::Value* of integer8 type with a value of zero.
    llvm::Value *GetZero_8(CodeGenerator *codegen);

    // Creates a LLVM::Value* of integer8 type with a value of one.
    llvm::Value *GetOne_8(CodeGenerator *codegen);

    // Creates a LLVM::Value* of integer8 type with a value of two.
    llvm::Value *GetTwo_8(CodeGenerator *codegen);
    
    // Creates a LLVM::Value* of integer1 type from the llvm::Value passed.
    llvm::Value *GetBoolean(CodeGenerator *codegen, bool val);
   
    // Creates a LLVM::Value* of integer64 type from the value passed.
    llvm::Value *GetInt64(CodeGenerator *codegen, unsigned long long int val);
   
    // Creates a LLVM::Value* of integer8* type from the llvm::Value passed.
    llvm::Value *GetString(CodeGenerator *codegen, std::string val);
  
    // Returns a string representation of the passed llvm type.
    std::string GetLLVMTypeName(llvm::Type *Ty);
  
    // Will attempt to cast one llvm::Value to another type and sets 'castSuccessful' to true if a cast happened, otherwise false.
    llvm::Value *CreateCastTo(CodeGenerator *codegen, llvm::Value *val, llvm::Type *castToType, bool *castSuccessful = nullptr);
  
    // Creates a default value for a given type, returns nullptr if default value isn't known.
    llvm::Value *GetDefaultValue(CodeGenerator *codegen, AstTypeNode *typeNode);
    llvm::Value *GetDefaultValue(CodeGenerator *codegen, llvm::Type *type);

    // Returns the current llvm::Function*
    llvm::Function *GetCurrentFunction(CodeGenerator *codegen);
    
    // Converts a Value* to a boolean.
    llvm::Value *ToBoolean(CodeGenerator *codegen, llvm::Value *num);
}

#endif