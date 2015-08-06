#include "llvm/IR/Module.h"
#include <stdarg.h>

#include "CodeGenerator.h"
#include "../Compiler/TreeContainer.h"
#include "CodeGeneratorHelpers.h"


using namespace llvm;
namespace Helpers {

    std::nullptr_t Error(PossiblePosition pos, const char *fmt, ...) {
        char buf[4096];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, 4096, fmt, args);
        va_end(args);
        fprintf(stderr, "(%d:%d) - Codegen Error: %s\n", pos.LineNumber, pos.ColumnNumber, buf);
        return nullptr;
    }

    void Warning(PossiblePosition pos, const char *fmt, ...) {
        char buf[4096];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, 4096, fmt, args);
        va_end(args);
        fprintf(stderr, "(%d:%d) - Codegen Warning: %s\n", pos.LineNumber, pos.ColumnNumber, buf);
    }

    namespace BinOperations {
        typedef std::map<Type::TypeID, std::map<Type::TypeID, GetFuncPtr > > LookupTableMapMap;

        Value *FailedLookupFunc(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return nullptr;
        }

        // int:int

        Value *intintAdd(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateAdd(lhs, rhs, "intintAdd");
        }
        Value *intintSub(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateSub(lhs, rhs, "intintSub");
        }
        Value *intintMul(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateMul(lhs, rhs, "intintMul");
        }
        Value *intintSDiv(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateSDiv(lhs, rhs, "intintDiv");
        }
        Value *intintSMod(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateSRem(lhs, rhs, "intintMod");
        }
        Value *intintUDiv(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateUDiv(lhs, rhs, "intintDiv");
        }
        Value *intintUMod(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateURem(lhs, rhs, "intintMod");
        }
        Value *intintAnd(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateAnd(lhs, rhs, "intintAnd");
        }
        Value *intintOr(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateOr(lhs, rhs, "intintOr");
        }
        Value *intintXor(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateXor(lhs, rhs, "intintXor");
        }
        Value *intintSHL(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateShl(lhs, rhs, "intintLSH");
        }
        Value *intintSHR(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateLShr(lhs, rhs, "intintRSH");
        }
        Value *intintSLT(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateICmpSLT(lhs, rhs, "intintLT");
        }
        Value *intintSGT(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateICmpSGT(lhs, rhs, "intintGT");
        }
        Value *intintSLE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateICmpSLE(lhs, rhs, "intintLE");
        }
        Value *intintSGE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateICmpSGE(lhs, rhs, "intintGE");
        }
        Value *intintULT(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateICmpULT(lhs, rhs, "intintLT");
        }
        Value *intintUGT(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateICmpUGT(lhs, rhs, "intintGT");
        }
        Value *intintULE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateICmpULE(lhs, rhs, "intintLE");
        }
        Value *intintUGE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateICmpUGE(lhs, rhs, "intintGE");
        }
        Value *intintEQ(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateICmpEQ(lhs, rhs, "intintEQ");
        }
        Value *intintNE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateICmpNE(lhs, rhs, "intintNE");
        }

        BinOpCodeGenFuncPtr getIntIntFuncPtr(TokenType oper) {
            switch (oper) {
            default: return FailedLookupFunc;
            case '+': return intintAdd;
            case '-': return intintSub;
            case '*': return intintMul;
            case '&': return intintAnd;
            case '|': return intintOr;
            case '^': return intintXor;
            case tok_leftshift: return intintSHL;
            case tok_rightshift: return intintSHR;
            case tok_equalequal: return intintEQ;
            case tok_notequal: return intintNE;
            }
        }
        BinOpCodeGenFuncPtr getIntIntSignedFuncPtr(TokenType oper) {
            switch (oper) {
            default: return getIntIntFuncPtr(oper);
            case '/': return intintSDiv;
            case '%': return intintSMod;
            case '<': return intintSLT;
            case '>': return intintSGT;
            case tok_lessequal: return intintSLE;
            case tok_greatequal: return intintSGE;
            }
        }

        BinOpCodeGenFuncPtr getIntIntUnsignedFuncPtr(TokenType oper) {
            switch (oper) {
            default: return getIntIntFuncPtr(oper);
            case '/': return intintUDiv;
            case '%': return intintUMod;
            case '<': return intintULT;
            case '>': return intintUGT;
            case tok_lessequal: return intintULE;
            case tok_greatequal: return intintUGE;
            }
        }

        // int:double | double:int
        Value *intToDouble(CodeGenerator *codegen, Value *intValue) {
            return codegen->getBuilder().CreateSIToFP(intValue, Type::getDoubleTy(codegen->getContext()), "uintToFp");
        }
        Value *doubleToInt(CodeGenerator *codegen, Value *doubleValue) {
            return codegen->getBuilder().CreateSIToFP(doubleValue, Type::getInt32Ty(codegen->getContext()), "fpToUint");
        }

        Value *intdoubleAdd(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy()) {
                lhs = intToDouble(codegen, lhs);
            }
            else {
                rhs = intToDouble(codegen, rhs);
            }

            return codegen->getBuilder().CreateFAdd(lhs, rhs, "intdoubleAdd");
        }
        Value *intdoubleSub(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy()) {
                lhs = intToDouble(codegen, lhs);
            }
            else {
                rhs = intToDouble(codegen, rhs);
            }

            return codegen->getBuilder().CreateFSub(lhs, rhs, "intdoubleSub");
        }
        Value *intdoubleMul(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy()) {
                lhs = intToDouble(codegen, lhs);
            }
            else {
                rhs = intToDouble(codegen, rhs);
            }

            return codegen->getBuilder().CreateFMul(lhs, rhs, "intdoubleMul");
        }
        Value *intdoubleDiv(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy()) {
                lhs = intToDouble(codegen, lhs);
            }
            else {
                rhs = intToDouble(codegen, rhs);
            }

            return codegen->getBuilder().CreateFDiv(lhs, rhs, "intdoubleDiv");
        }
        Value *intdoubleMod(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy()) {
                lhs = intToDouble(codegen, lhs);
            }
            else {
                rhs = intToDouble(codegen, rhs);
            }

            return codegen->getBuilder().CreateFRem(lhs, rhs, "intdoubleMod");
        }
        Value *intdoubleLT(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy()) {
                lhs = intToDouble(codegen, lhs);
            }
            else {
                rhs = intToDouble(codegen, rhs);
            }

            return codegen->getBuilder().CreateFCmpULT(lhs, rhs, "intdoubleLT");
        }
        Value *intdoubleGT(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy()) {
                lhs = intToDouble(codegen, lhs);
            }
            else {
                rhs = intToDouble(codegen, rhs);
            }

            return codegen->getBuilder().CreateFCmpUGT(lhs, rhs, "intdoubleGT");
        }
        Value *intdoubleLE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy()) {
                lhs = intToDouble(codegen, lhs);
            }
            else {
                rhs = intToDouble(codegen, rhs);
            }

            return codegen->getBuilder().CreateFCmpULE(lhs, rhs, "intdoubleLE");
        }
        Value *intdoubleGE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy()) {
                lhs = intToDouble(codegen, lhs);
            }
            else {
                rhs = intToDouble(codegen, rhs);
            }

            return codegen->getBuilder().CreateFCmpUGE(lhs, rhs, "intdoubleGE");
        }
        Value *intdoubleEQ(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy()) {
                lhs = intToDouble(codegen, lhs);
            }
            else {
                rhs = intToDouble(codegen, rhs);
            }

            return codegen->getBuilder().CreateFCmpOEQ(lhs, rhs, "intdoubleEQ");
        }
        Value *intdoubleNE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            if (lhs->getType()->isIntegerTy()) {
                lhs = intToDouble(codegen, lhs);
            }
            else {
                rhs = intToDouble(codegen, rhs);
            }

            return codegen->getBuilder().CreateFCmpONE(lhs, rhs, "intdoubleNE");
        }

        BinOpCodeGenFuncPtr getIntDoubleFuncPtr(TokenType oper) {
            switch (oper) {
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
            return codegen->getBuilder().CreateFAdd(lhs, rhs, "doubledoubleAdd");
        }
        Value *doubledoubleSub(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateFSub(lhs, rhs, "doubledoubleSub");
        }
        Value *doubledoubleMul(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateFMul(lhs, rhs, "doubledoubleMul");
        }
        Value *doubledoubleDiv(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateFDiv(lhs, rhs, "doubledoubleDiv");
        }
        Value *doubledoubleMod(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateFRem(lhs, rhs, "doubledoubleMod");
        }
        Value *doubledoubleLT(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateFCmpULT(lhs, rhs, "doubledoubleLT");
        }
        Value *doubledoubleGT(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateFCmpUGT(lhs, rhs, "doubledoubleGT");
        }
        Value *doubledoubleLE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateFCmpULE(lhs, rhs, "doubledoubleLE");
        }
        Value *doubledoubleGE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateFCmpUGE(lhs, rhs, "doubledoubleGE");
        }
        Value *doubledoubleEQ(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateFCmpOEQ(lhs, rhs, "doubledoubleEQ");
        }
        Value *doubledoubleNE(CodeGenerator *codegen, Value *lhs, Value *rhs) {
            return codegen->getBuilder().CreateFCmpONE(lhs, rhs, "doubledoubleNE");
        }

        BinOpCodeGenFuncPtr getDoubleDoubleFuncPtr(TokenType oper) {
            switch (oper) {
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

        LookupTableMapMap getBinOpLookupTable(bool isUnsigned) {
            LookupTableMapMap lookupTable;
            lookupTable[Type::IntegerTyID][Type::IntegerTyID] = isUnsigned ? getIntIntUnsignedFuncPtr : getIntIntSignedFuncPtr;
            lookupTable[Type::IntegerTyID][Type::DoubleTyID] = getIntDoubleFuncPtr;
            lookupTable[Type::DoubleTyID][Type::IntegerTyID] = getIntDoubleFuncPtr;
            lookupTable[Type::DoubleTyID][Type::DoubleTyID] = getDoubleDoubleFuncPtr;
            return lookupTable;
        }
    }
    // Returns a pointer to a function which will generate the proper LLVM code to handle the operator and types passed.
    BinOperations::BinOpCodeGenFuncPtr GetBinopCodeGenFuncPointer(TokenType Operator, Type *lType, Type *rType, bool isUnsigned) {
        auto func = BinOperations::getBinOpLookupTable(isUnsigned)[lType->getTypeID()][rType->getTypeID()];
        if (func == nullptr) {
            return nullptr;
        }
        return func(Operator);
    }

    // EmitScopeBlock - Emits a std::vector of ast::IAstExpr* expressions and returns their value
    // in a std::vector<Value*>
    std::vector<Value*> EmitScopeBlock(CodeGenerator *codegen, const std::vector<IAstExpression*> &block, bool stopAtFirstReturn, bool *stopped) {
        unsigned varCount = codegen->getVarCount();
        codegen->incrementNestDepth();
        std::vector<Value*> vals;
        for (unsigned i = 0, size = block.size(); i < size; ++i) {
            IAstExpression *expr = block[i];
            if (codegen->getBuilder().GetInsertBlock()->getTerminator() != nullptr) // stops multiple terminators per block.
            {
                Warning(expr->getPos(), "Unreachable code.");
                continue;
            }
            Value *val = expr->Codegen(codegen);
            if (val == nullptr) {
                vals.clear();
                goto RETURN;
            }
            vals.push_back(val);
            if (stopAtFirstReturn && expr->getNodeType() == AstNodeType::node_return) {
                if (stopped != nullptr) { *stopped = true; }
                goto RETURN;
            }
        }
        if (stopped != nullptr) { *stopped = false; }
    RETURN:
        // Pops all variables declared within this scope
        codegen->popFromScopeStack(codegen->getVarCount() - varCount);
        codegen->decrementNestDepth();
        return vals;
    }
    // Create an alloca instruction in the entry block of the function.  This is used for mutable variables etc.
    AllocaInst *CreateEntryBlockAlloca(CodeGenerator *codegen, Function *function, const std::string &varName, Type *type) {
        BasicBlock *prevInsertPoint = codegen->getBuilder().GetInsertBlock();
        codegen->getBuilder().SetInsertPoint(&function->getEntryBlock(), function->getEntryBlock().begin());
        AllocaInst *Alloca = codegen->getBuilder().CreateAlloca(type, nullptr, varName.c_str());
        codegen->getBuilder().SetInsertPoint(prevInsertPoint);
        return Alloca;
    }

    // Links blocks without a terminator to the next block
    void LinkBlocksWithoutTerminator(CodeGenerator *codegen, Function *function) {
        auto bb = function->getBasicBlockList().begin();
        auto next = bb;
        auto end = function->getBasicBlockList().end();
        // Save the current insert block
        BasicBlock *restorePoint = codegen->getBuilder().GetInsertBlock();
        for (; bb != end && ++next != end; ++bb) {
            if (bb->getTerminator() == nullptr) {
                codegen->getBuilder().SetInsertPoint(bb);
                codegen->getBuilder().CreateBr(next);
            }
        }
        // Restore the insert point position.
        codegen->getBuilder().SetInsertPoint(restorePoint);
    }

    // Creates a Value* of double type from the value passed.
    Value *GetDouble(CodeGenerator *codegen, double val) {
        return ConstantFP::get(Type::getDoubleTy(codegen->getContext()), val);
    }
    // Creates a Value* of float type from the Value passed.
    Value *GetFloat(CodeGenerator *codegen, float val) {
        return ConstantFP::get(Type::getFloatTy(codegen->getContext()), val);
    }
    // Creates a Value* of integer64 type with a value of zero.
    Value *GetZero_64(CodeGenerator *codegen) {
        return GetInt64(codegen, 0);
    }
    // Creates a Value* of integer64 type with a value of one.
    Value *GetOne_64(CodeGenerator *codegen) {
        return GetInt64(codegen, 1);
    }
    // Creates a Value* of integer64 type with a value of two.
    Value *GetTwo_64(CodeGenerator *codegen) {
        return GetInt64(codegen, 2);
    }
    // Creates a Value* of integer32 type with a value of zero.
    Value *GetZero_32(CodeGenerator *codegen) {
        return GetInt32(codegen, 0);
    }
    // Creates a Value* of integer32 type with a value of one.
    Value *GetOne_32(CodeGenerator *codegen) {
        return GetInt32(codegen, 1);
    }
    // Creates a Value* of integer32 type with a value of two.
    Value *GetTwo_32(CodeGenerator *codegen) {
        return GetInt32(codegen, 2);
    }
    // Creates a Value* of integer16 type with a value of zero.
    Value *GetZero_16(CodeGenerator *codegen) {
        return GetInt16(codegen, 2);
    }
    // Creates a Value* of integer16 type with a value of one.
    Value *GetOne_16(CodeGenerator *codegen) {
        return GetInt16(codegen, 2);
    }
    // Creates a Value* of integer16 type with a value of two.
    Value *GetTwo_16(CodeGenerator *codegen){
        return GetInt16(codegen, 2);
    }
    // Creates a Value* of integer8 type with a value of zero.
    Value *GetZero_8(CodeGenerator *codegen) {
        return GetInt8(codegen, 0);
    }
    // Creates a Value* of integer8 type with a value of one.
    Value *GetOne_8(CodeGenerator *codegen) {
        return GetInt8(codegen, 1);
    }
    // Creates a Value* of integer8 type with a value of two.
    Value *GetTwo_8(CodeGenerator *codegen) {
        return GetInt8(codegen, 2);
    }
    // Normalizes integer bitwidths, basically just casts up to the larger.
    void NormalizeIntegerWidths(CodeGenerator *codegen, Value *lhs, Value *rhs) {
        Type *lType = lhs->getType();
        Type *rType = rhs->getType();
        // If either type is not an integer or they are already the same size, return with no change.
        if (!lType->isIntegerTy() || !rType->isIntegerTy() || lType->getIntegerBitWidth() == rType->getIntegerBitWidth()) {
            return;
        }

        if (lType->getIntegerBitWidth() > rType->getIntegerBitWidth()) { // lhs has higher bit width, cast rhs to lhs size.
            rhs = CreateCastTo(codegen, rhs, lType);
            //rhs = codegen->getBuilder().CreateIntCast(rhs, lType,)
        }
        else { // rhs has higher bit width, cast lhs to rhs size.
            lhs = CreateCastTo(codegen, lhs, rType);
        }
    }

    // Creates a Value* of integer type with specified width from the value passed.
    Value *GetInt(CodeGenerator *codegen, demi_int val, int bitwidth) {
        return ConstantInt::get(codegen->getContext(), APInt(bitwidth, val, true));
    }
    // Creates a Value* of integer type with specified width from the value passed.
    Value *GetUInt(CodeGenerator *codegen, demi_int val, int bitwidth) {
        return ConstantInt::get(codegen->getContext(), APInt(bitwidth, val, false));
    }
    // Creates a Value* of the demiurge unsigned integer size.
    // This should be used for any generic number type such as constants
    Value *GetDemiUInt(CodeGenerator *codegen, demi_int val) {
        return GetUInt(codegen, val, 32);
    }
    // Creates a Value* of the demiurge unsigned integer size.
    // This should be used for any generic number type such as constants
    Value *GetDemiInt(CodeGenerator *codegen, demi_int val) {
        return GetInt(codegen, val, 32);
    }
    // Creates a Value* of integer1 type from the value passed.
    Value *GetBoolean(CodeGenerator *codegen, int val) {
        return ConstantInt::get(Type::getInt1Ty(codegen->getContext()), APInt(1, val));
    }
    // Creates a Value* of signed integer64 type from the value passed.
    Value *GetInt64(CodeGenerator *codegen, demi_int val) {
        return GetInt(codegen, val, 64);
    }
    // Creates a Value* of signed integer32 type from the value passed.
    Value *GetInt32(CodeGenerator *codegen, demi_int val) {
        return GetInt(codegen, val, 32);
    }
    // Creates a Value* of signed integer16 type from the value passed.
    Value *GetInt16(CodeGenerator *codegen, demi_int val) {
        return GetInt(codegen, val, 16);
    }
    // Creates a Value* of signed integer8 type from the value passed.
    Value *GetInt8(CodeGenerator *codegen, demi_int val) {
        return GetInt(codegen, val, 8);
    }
    // Creates a Value* of unsigned integer64 type from the value passed.
    Value *GetUInt64(CodeGenerator *codegen, demi_int val) {
        return GetUInt(codegen, val, 64);
    }
    // Creates a Value* of unsigned integer32 type from the value passed.
    Value *GetUInt32(CodeGenerator *codegen, demi_int val) {
        return GetUInt(codegen, val, 32);
    }
    // Creates a Value* of unsigned integer16 type from the value passed.
    Value *GetUInt16(CodeGenerator *codegen, demi_int val) {
        return GetUInt(codegen, val, 16);
    }
    // Creates a Value* of unsigned integer8 type from the value passed.
    Value *GetUInt8(CodeGenerator *codegen, demi_int val) {
        return GetUInt(codegen, val, 8);
    }
    // Creates a Value* of integer8* type from the value passed.
    Value *GetString(CodeGenerator *codegen, std::string val) {
        return ConstantDataArray::getString(codegen->getContext(), val.c_str());
    }


    // Returns true if the value is a pointer to a pointer
    bool IsPtrToPtr(Value *val) {
        return val->getType()->isPointerTy() && val->getType()->getContainedType(0)->getTypeID() == Type::TypeID::PointerTyID;
    }

    // Returns true if the value is a pointer to an array
    bool IsPtrToArray(Value *val) {
        return val->getType()->isPointerTy() && val->getType()->getContainedType(0)->isArrayTy();
    }

    // Returns whether a value is a number.
    bool IsNumberType(Value *val) {
        return val->getType()->isIntegerTy() || val->getType()->isFloatingPointTy();
    }

    // Returns whether a value is an integer but not a boolean, basically bitwidth > 1.
    bool IsNonBooleanIntegerType(Value *val) {
        return val->getType()->isIntegerTy() && !val->getType()->isIntegerTy(1);
    }

    // Returns whether a value is a number that is not a boolean.
    bool IsNonBooleanNumberType(Value *val) {
        return val->getType()->isFloatingPointTy() || IsNonBooleanIntegerType(val);
    }

    // Returns whether a node type is unsigned.
    bool IsUnsigned(AstNodeType type) {
        switch (type) {
        default: return false;
        case node_unsigned_integer8:
        case node_unsigned_integer16:
        case node_unsigned_integer32:
        case node_unsigned_integer64:
            return true;
        }
    }

    // Returns a string representation of the passed llvm type.
    std::string GetLLVMTypeName(Type *Ty) {
        switch (Ty->getTypeID()) {
        case Type::TypeID::VoidTyID: return "void";                     //  0: type with no size
        case Type::TypeID::FloatTyID: return "float";                 //  2: 32-bit floating point type
        case Type::TypeID::DoubleTyID: return "double";                //  3: 64-bit floating point type

        case Type::TypeID::IntegerTyID: return "integer";               // 10: Arbitrary bit width integers
        case Type::TypeID::FunctionTyID: return "function";             // 11: Functions
        case Type::TypeID::StructTyID: return "struct";                 // 12: Structures
        case Type::TypeID::ArrayTyID: return "array";                   // 13: Arrays
        case Type::TypeID::PointerTyID: return "pointer";               // 14: Pointers
        case Type::TypeID::VectorTyID: return "vector";                 // 15: SIMD 'packed' format: or other vector type

        case Type::TypeID::LabelTyID: return "label";                   //  7: Labels
        case Type::TypeID::MetadataTyID: return "metadata";             //  8: Metadata
        //  Likely to never be used:
        //case Type::TypeID::HalfTyID: return "float16";                  //  1: 16-bit floating point type
        //case Type::TypeID::X86_FP80TyID: return "float80_x87";          //  4: 80-bit floating point type (X87)
        //case Type::TypeID::FP128TyID: return "float128_112bit_mantissa";//  5: 128-bit floating point type (112-bit mantissa)
        //case Type::TypeID::PPC_FP128TyID: return "float128_ppc";        //  6: 128-bit floating point type (two 64-bits: PowerPC)
        //case Type::TypeID::X86_MMXTyID: return "mmx_vector_64_x86";     //  9: MMX vectors (64 bits: X86 specific)
        }
    }

    // Returns the pointer to the first element of an array.
    Value *CreateArrayDecay(CodeGenerator *codegen, Value *val) {
        Value *zero = GetDemiUInt(codegen, 0);
        Value *arrayref[] = { zero, zero };
        return codegen->getBuilder().CreateGEP(val, arrayref, "arraydecay");
    }

    // Will attempt to cast one value to another type and sets 'castSuccessful' to true if a cast happened, otherwise false.
    Value *CreateCastTo(CodeGenerator *codegen, Value *val, Type *castToType, bool *castSuccessful) {
        if (castSuccessful != nullptr) {
            *castSuccessful = false;
        }
        Type *valType = val->getType();
        if (valType->isIntegerTy() && castToType->isIntegerTy()) {
            unsigned int fromWidth = val->getType()->getIntegerBitWidth();
            unsigned int toWidth = castToType->getIntegerBitWidth();
            if (fromWidth != toWidth) { // resize the width of the the val
                if (castSuccessful != nullptr) {
                    *castSuccessful = true;
                }
                return codegen->getBuilder().CreateCast(CastInst::getCastOpcode(val, true, castToType, true), val, castToType, "castto");
            }
        }
        auto valTypeId = valType->getTypeID();
        auto castTypeId = castToType->getTypeID();
        if (valTypeId == castTypeId) {// already same type, just return what was sent
            return val;
        }
        if (valType->canLosslesslyBitCastTo(castToType)) { // bitcasting
            if (castSuccessful != nullptr) {
                *castSuccessful = true;
            }
            return codegen->getBuilder().CreateBitCast(val, castToType, "bitcasted");
        }
        if (CastInst::isCastable(valType, castToType)) { // typical cast to type
            if (castSuccessful != nullptr) {
                *castSuccessful = true;
            }
            return codegen->getBuilder().CreateCast(CastInst::getCastOpcode(val, true, castToType, true), val, castToType, "castto");
        }
        return val; // if we hit this fast, we failed to cast so just return what was passed.
    }

    // Used to implicitly cast numbers such as integers and doubles but not pointers.
    Value *CreateImplicitCast(CodeGenerator *codegen, Value *val, Type *castToType, bool *castSuccessful) {
        if (IsNumberType(val)) { // only implicityly cast number.
            return CreateCastTo(codegen, val, castToType, castSuccessful);
        }
        return val;
    }

    Value *GetDefaultValue(CodeGenerator *codegen, AstTypeNode *typeNode) {
        switch (typeNode->getTypeType()) {
        default: return nullptr;
        case node_boolean: return codegen->getBuilder().getFalse();
        case node_float: return GetFloat(codegen, 0.0f);
        case node_double: return GetDouble(codegen, 0.0);

        case node_signed_integer8: return GetInt8(codegen, 0);
        case node_signed_integer16: return GetInt16(codegen, 0);
        case node_signed_integer32: return GetInt32(codegen, 0);
        case node_signed_integer64: return GetInt64(codegen, 0);

        case node_unsigned_integer8: return GetUInt8(codegen, 0);
        case node_unsigned_integer16: return GetUInt16(codegen, 0);
        case node_unsigned_integer32: return GetUInt32(codegen, 0);
        case node_unsigned_integer64: return GetUInt64(codegen, 0);

        case node_string: return GetString(codegen, "");
        }
    }

    Value *GetDefaultValue(CodeGenerator *codegen, Type *type) {
        switch (type->getTypeID()) {
        default: return nullptr;
        case Type::PointerTyID: return ConstantPointerNull::get(0);
        case Type::DoubleTyID: return GetDouble(codegen, 0.0);
        case Type::FloatTyID: return GetFloat(codegen, 0.0f);
        case Type::IntegerTyID: return GetUInt32(codegen, 0);
        }
    }

    Value *ToBoolean(CodeGenerator *codegen, Value *num) {
        return codegen->getBuilder().CreateICmpNE(num, codegen->getBuilder().getFalse(), "tobool");
    }

    // Creates an ID to append to a label for easier nesting readability
    std::string MakeLabelId() {
        std::string labelId;
        
        for (char i = 0; i < 5; ++i) {
            labelId += (char)((rand() % 26) + 65);
        }
        return labelId;
    }

    // Creates a call to calloc
    Value *CreateCallocCall(CodeGenerator *codegen, Type *type, Value *numOfItems) {
        return nullptr;
    }
    // Creates a call to malloc
    Value *CreateMallocCall(CodeGenerator *codegen, Value *size) {
        /*Function *Malloc= codegen->getTheModule()->getOrInsertFunction("malloc", FunctionType::get(;
        codegen->getBuilder().CreateCall()*/
        return nullptr;
    }
    // Creates a call to free
    Value *CreateFreeCall(CodeGenerator *codegen, Value *ptr) {
        return nullptr;
    }
}