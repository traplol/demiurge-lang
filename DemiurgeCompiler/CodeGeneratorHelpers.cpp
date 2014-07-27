#include "llvm/IR/Module.h"
#include <stdarg.h>

#include "CodeGenerator.h"
#include "TreeContainer.h"
#include "CodeGeneratorHelpers.h"

using namespace llvm;
namespace Helpers {

    nullptr_t Error(PossiblePosition pos, const char *fmt, ...) {
        char buf[4096];
        va_list args;
        va_start(args, fmt);
        vsnprintf_s(buf, 4096, fmt, args);
        va_end(args);
        fprintf(stderr, "(%d:%d) - Codegen Error: %s\n", pos.LineNumber, pos.ColumnNumber, buf);
        return nullptr;
    }

    void Warning(PossiblePosition pos, const char *fmt, ...) {
        char buf[4096];
        va_list args;
        va_start(args, fmt);
        vsnprintf_s(buf, 4096, fmt, args);
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
    BinOperations::BinOpCodeGenFuncPtr GetBinopCodeGenFuncPointer(TokenType Operator, Type *lType, Type *rType) {
        auto func = BinOperations::getBinOpLookupTable()[lType->getTypeID()][rType->getTypeID()];
        if (func == nullptr) return nullptr;
        return func(Operator);
    }

    // EmitBlock - Emits a std::vector of ast::IAstExpr* expressions and returns their value
    // in a std::vector<llvm::Value*>
    std::vector<Value*> EmitBlock(CodeGenerator *codegen, const std::vector<IExpressionAST*> &block, bool stopAtFirstReturn, bool *stopped) {
        std::vector<Value*> vals;
        for (unsigned i = 0, size = block.size(); i < size; ++i) {
            IExpressionAST *expr = block[i];
            if (codegen->Builder.GetInsertBlock()->getTerminator() != nullptr) // stops multiple terminators per block.
            {
                Warning(expr->getPos(), "Unreachable code.");
                continue;
            }
            
            Value *val = expr->Codegen(codegen);
            if (val == nullptr) {
                vals.clear();
                return vals;
            }
            vals.push_back(val);
            if (stopAtFirstReturn && expr->getNodeType() == AstNodeType::node_return) {
                if (stopped != nullptr) { *stopped = true; }
                return vals;
            }
        }
        if (stopped != nullptr) { *stopped = false; }
        return vals;
    }
    // CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
    // the function.  This is used for mutable variables etc.
    AllocaInst* CreateEntryBlockAlloca(Function *function, const std::string &varName, Type *type) {
        IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
        return tmpBuilder.CreateAlloca(type, 0, varName.c_str());
    }

    // Creates a LLVM::Value* of double type from the value passed.
    Value *GetDouble(CodeGenerator *codegen, double val) {
        return ConstantFP::get(codegen->Context, APFloat(val));
    }
    // Creates a LLVM::Value* of integer64 type with a value of zero.
    Value *GetZero_64(CodeGenerator *codegen) {
        return GetInt64(codegen, 0);
    }
    // Creates a LLVM::Value* of integer64 type with a value of one.
    Value *GetOne_64(CodeGenerator *codegen) {
        return GetInt64(codegen, 1);
    }
    // Creates a LLVM::Value* of integer64 type with a value of two.
    Value *GetTwo_64(CodeGenerator *codegen) {
        return GetInt64(codegen, 2);
    }
    // Creates a LLVM::Value* of integer8 type with a value of zero.
    llvm::Value *GetZero_8(CodeGenerator *codegen) {
        return GetInt(codegen, 0, 0);
    }
    // Creates a LLVM::Value* of integer8 type with a value of one.
    llvm::Value *GetOne_8(CodeGenerator *codegen) {
        return GetInt(codegen, 1, 8);
    }
    // Creates a LLVM::Value* of integer8 type with a value of two.
    llvm::Value *GetTwo_8(CodeGenerator *codegen) {
        return GetInt(codegen, 2, 8);
    }

    // Creates a LLVM::Value* of integer type with specified width from the value passed.
    Value *GetInt(CodeGenerator *codegen, unsigned long long int val, int bitwidth) {
        return ConstantInt::get(codegen->Context, APInt(bitwidth, val));
    }
    // Creates a LLVM::Value* of integer1 type from the value passed.
    Value *GetBoolean(CodeGenerator *codegen, int val) {
        return ConstantInt::get(Type::getInt1Ty(codegen->Context), APInt(1, val));
    }
    // Creates a LLVM::Value* of integer64 type from the value passed.
    Value *GetInt64(CodeGenerator *codegen, unsigned long long int val) {
        return GetInt(codegen, val, 64);
    }
    // Creates a LLVM::Value* of integer8* type from the value passed.
    Value *GetString(CodeGenerator *codegen, std::string val) {
        return ConstantDataArray::getString(codegen->Context, val.c_str());
    }

    // Returns a string representation of the passed llvm type.
    std::string GetLLVMTypeName(Type *Ty) {
        switch (Ty->getTypeID()) {
        case Type::TypeID::VoidTyID: return "void";                     //  0: type with no size
        case Type::TypeID::FloatTyID: return "float32";                 //  2: 32-bit floating point type
        case Type::TypeID::DoubleTyID: return "float64";                //  3: 64-bit floating point type

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

    // Will attempt to cast one value to another type and sets 'castSuccessful' to true if a cast happened, otherwise false.
    Value *CreateCastTo(CodeGenerator *codegen, Value *val, Type *castToType, bool *castSuccessful) {
        if (castSuccessful != nullptr)
            *castSuccessful = false;
        Type *valType = val->getType();
        if (valType->isIntegerTy() && castToType->isIntegerTy()) {
            unsigned int fromWidth = val->getType()->getIntegerBitWidth();
            unsigned int toWidth = castToType->getIntegerBitWidth();
            if (fromWidth != toWidth) { // resize the width of the the val
                if (castSuccessful != nullptr)
                    *castSuccessful = true;
                return codegen->Builder.CreateCast(CastInst::getCastOpcode(val, true, castToType, true), val, castToType, "castto");
            }
        }
        auto valTypeId = valType->getTypeID();
        auto castTypeId = castToType->getTypeID();
        if (valTypeId == castTypeId) // already same type, just return what was sent
            return val;
        if (valType->canLosslesslyBitCastTo(castToType)) { // bitcasting
            if (castSuccessful != nullptr)
                *castSuccessful = true;
            return codegen->Builder.CreateBitCast(val, castToType, "bitcasted");
        }
        if (CastInst::isCastable(valType, castToType)) { // typical cast to type
            if (castSuccessful != nullptr)
                *castSuccessful = true;
            return codegen->Builder.CreateCast(CastInst::getCastOpcode(val, true, castToType, true), val, castToType, "castto");
        }
        return val;
    }

    Value *GetDefaultValue(CodeGenerator *codegen, AstTypeNode *typeNode) {
        switch (typeNode->getTypeType()) {
        default: return nullptr;
        case node_boolean: return codegen->Builder.getFalse();
        case node_double: return GetDouble(codegen, 0.0);
        case node_integer: return GetInt64(codegen, 0);
        case node_string: return GetString(codegen, "");
        }
    }

    Value *GetDefaultValue(CodeGenerator *codegen, Type *type) {
        switch (type->getTypeID()) {
        default: return nullptr;
        case Type::PointerTyID: return ConstantPointerNull::get(0);
        case Type::DoubleTyID: return GetDouble(codegen, 0.0);
        case Type::IntegerTyID: return GetInt64(codegen, 0);
        }
    }

    // Returns the current llvm::Function*
    Function *GetCurrentFunction(CodeGenerator *codegen) {
        return codegen->Builder.GetInsertBlock()->getParent();
    }
}