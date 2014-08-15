#include "AstUnaryOperatorExpr.h"
#include "AstIntegerNode.h"
#include "AstBinaryOperatorExpr.h"

#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"

#include "llvm/IR/Module.h"

using namespace llvm;

AstUnaryOperatorExpr::AstUnaryOperatorExpr(const std::string &operStr, TokenType oper, IAstExpression *operand, bool isPostfix, int line, int column) {
    init(operStr, oper, operand, isPostfix, nullptr, line, column, nullptr);
}

AstUnaryOperatorExpr::AstUnaryOperatorExpr(const std::string &operStr, TokenType oper, IAstExpression *operand, bool isPostfix,
    IAstExpression *index, int line, int column) {
    init(operStr, oper, operand, isPostfix, index, line, column, nullptr);
}
AstUnaryOperatorExpr::AstUnaryOperatorExpr(const std::string &operStr, TokenType oper, AstTypeNode *type, bool isPostfix, int line, int column) {
    init(operStr, oper, nullptr, IsPostfix, nullptr, line, column, type);
}
AstUnaryOperatorExpr::~AstUnaryOperatorExpr() {
    delete Operand, IndexExpr, TypeNode;
}

void AstUnaryOperatorExpr::init(const std::string &operStr, TokenType oper, IAstExpression *operand, bool isPostfix,
    IAstExpression *index, int line, int column, AstTypeNode *type) {
    OperatorString = operStr;
    Operator = oper;
    Operand = operand;
    IsPostfix = isPostfix;
    IndexExpr = index;
    TypeNode = type;
    setNodeType(isPostfix ? node_unary_operation_post : node_unary_operation_pre);
    setPos(PossiblePosition{ line, column });
}

bool AstUnaryOperatorExpr::getIsPostfix() const {
    return IsPostfix;
}
bool AstUnaryOperatorExpr::getIsPrefix() const {
    return !IsPostfix;
}

llvm::Value *AstUnaryOperatorExpr::Codegen(CodeGenerator *codegen) {
    /*
    Unary Operators:
    '-', '+', '!', '~', '++', '--' '['<expression>']'
    */
    switch (this->Operator) {
    default: return nullptr;
    case '+': return positive(codegen);
    case '-': return negative(codegen);
    case '!': return logicNegate(codegen);
    case '~': return onesComplement(codegen);
    case tok_plusplus: return increment(codegen);
    case tok_minusminus: return decrement(codegen);
    case tok_new: return newMalloc(codegen);
    case '[': return accessElement(codegen);
    }
}

Value *AstUnaryOperatorExpr::positive(CodeGenerator *codegen) {
    Value *v = this->Operand->Codegen(codegen);
    if (!Helpers::IsNumberType(v)) {// cannot make a non-number positive
        return Helpers::Error(this->getPos(), "Value is not a number type.");
    }
    return v; // unary positive does not change the sign.
}
Value *AstUnaryOperatorExpr::negative(CodeGenerator *codegen) {
    Value *v = this->Operand->Codegen(codegen);
    
    if (v->getType()->isFloatingPointTy()) {
        return codegen->getBuilder().CreateFNeg(v, "fneg");
    }
    if (v->getType()->isIntegerTy()) {
        return codegen->getBuilder().CreateNeg(v, "neg");
    }

    return Helpers::Error(this->getPos(), "Value is not a number type."); // cannot make a non-number negative.
}
Value *AstUnaryOperatorExpr::logicNegate(CodeGenerator *codegen) {
    Value *v = this->Operand->Codegen(codegen);
    if (!v->getType()->isIntegerTy(1)) { // not int1 (bool)
        return Helpers::Error(this->getPos(), "Value is not bool type.");
    }
    return codegen->getBuilder().CreateNot(v, "negate");
}
Value *AstUnaryOperatorExpr::onesComplement(CodeGenerator *codegen) {
    Value *v = this->Operand->Codegen(codegen);
    if (!v->getType()->isIntegerTy()) { // not integer
        return Helpers::Error(this->getPos(), "Value is not integer type.");
    }
    return codegen->getBuilder().CreateNot(v, "negate");
}
Value *AstUnaryOperatorExpr::increment(CodeGenerator *codegen) {
    // We'll just expand increment into what it actually is:
    // x = x + 1; -> prefix
    // OR
    // x; -> postfix
    // x = x + 1;
    int line = this->getPos().LineNumber;
    int column = this->getPos().ColumnNumber;
    // The RHS of the add-by-one
    AstIntegerNode *rhs_one = new AstIntegerNode(1, line, column);
    
    AstBinaryOperatorExpr *expr = new AstBinaryOperatorExpr("+=", tok_plusequals, this->Operand, rhs_one, line, column);
    if (this->getIsPrefix()) {
        return expr->Codegen(codegen);
    }
    // we'll go ahead and evaluate the operand to return and then increment it.
    Value *val = this->Operand->Codegen(codegen);
    expr->Codegen(codegen);
    return val;
}
Value *AstUnaryOperatorExpr::decrement(CodeGenerator *codegen) {
    // We'll just expand decrement into what it actually is:
    // x = x - 1; -> prefix
    // OR
    // x; -> postfix
    // x = x - 1;
    int line = this->getPos().LineNumber;
    int column = this->getPos().ColumnNumber;
    // The RHS of the sub-by-one
    AstIntegerNode *rhs_one = new AstIntegerNode(1, line, column);

    AstBinaryOperatorExpr *expr = new AstBinaryOperatorExpr("-=", tok_minusequals, this->Operand, rhs_one, line, column);
    if (this->getIsPrefix()) {
        return expr->Codegen(codegen);
    }
    // we'll go ahead and evaluate the operand to return and then increment it.
    Value *val = this->Operand->Codegen(codegen);
    expr->Codegen(codegen);
    return val;
}
Value *AstUnaryOperatorExpr::accessElement(CodeGenerator *codegen) {
    Value *operand = this->Operand->Codegen(codegen);
    Value *idx = this->IndexExpr->Codegen(codegen);
    Value *gepzero = Helpers::GetDemiUInt(codegen, 0);
    Value *gepaddr;
    Value *arrayRef[] = { gepzero, idx };
    if (Helpers::IsPtrToArray(operand)) {
        gepaddr = codegen->getBuilder().CreateGEP(operand, arrayRef, "arrayidx");
    }
    else {
        gepaddr = codegen->getBuilder().CreateGEP(operand, idx, "arrayidx");
    }
    return codegen->getBuilder().CreateLoad(gepaddr);
}

Value *AstUnaryOperatorExpr::ArrayAssignment(CodeGenerator *codegen, IAstExpression *rhs) {
    Value *val = rhs->Codegen(codegen);
    Value *operand = this->Operand->Codegen(codegen);
    Value *idx = this->IndexExpr->Codegen(codegen);
    Value *gepzero = Helpers::GetDemiUInt(codegen, 0);
    Value *gepaddr;
    Value *arrayRef[] = { gepzero, idx };
    if (Helpers::IsPtrToArray(operand)) {
        gepaddr = codegen->getBuilder().CreateGEP(operand, arrayRef, "arrayidx");
    }
    else {
        gepaddr = codegen->getBuilder().CreateGEP(operand, idx, "arrayidx");
    }
    return codegen->getBuilder().CreateStore(val, gepaddr);
}

Value *AstUnaryOperatorExpr::newMalloc(CodeGenerator *codegen) {
    Type *type = this->TypeNode->GetLLVMType(codegen);
    size_t size = codegen->getTheModule()->getDataLayout()->getTypeAllocSize(type);

    //codegen->getBuilder().Create

    //malloc(size);

    return nullptr;
}