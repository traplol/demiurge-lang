//#include "llvm/IR/Type.h"
//#include "llvm/IR/Value.h"

#include "AstBinaryOperatorExpr.h"
#include "AstUnaryOperatorExpr.h"
#include "AstVariableNode.h"
#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"

using namespace llvm;

AstBinaryOperatorExpr::AstBinaryOperatorExpr(const std::string &operStr, TokenType oper, IAstExpression *lhs,
    IAstExpression *rhs, int line, int column)
    : OperatorString(operStr)
    , Operator(oper)
    , LHS(lhs)
    , RHS(rhs) {
    setNodeType(node_binary_operation);
    setPos(PossiblePosition{ line, column });
}
AstBinaryOperatorExpr::~AstBinaryOperatorExpr() {
    delete this->LHS, this->RHS;
}

Value *AstBinaryOperatorExpr::VariableAssignment(CodeGenerator *codegen) {
    if (this->LHS == nullptr) {
        return nullptr;
    }

    if (AstUnaryOperatorExpr *unary = dynamic_cast<AstUnaryOperatorExpr*>(this->LHS)) {
        if (unary->getOperator() == '[') {
            return unary->ArrayAssignment(codegen, this->RHS); // Array assignment.
        }
        else {
            //return unary->Codegen(codegen);
        }
    }

    AstVariableNode *lhse = dynamic_cast<AstVariableNode*>(this->LHS);
    if (lhse == nullptr) { // left side of assign operator isn't a variable.
        return Helpers::Error(this->LHS->getPos(), "Destination of assignment operator must be variable.");
    }

    // Codegen to evaluate the expression to store within the variable.
    Value *val = this->RHS->Codegen(codegen);
    if (val == nullptr) {
        return Helpers::Error(this->RHS->getPos(), "Right operand could not be evaluated.");
    }

    // Lookup the name of the variable
    
    Value *variable = codegen->getNamedValue(lhse->getName());
    if (variable == nullptr) {
        return Helpers::Error(this->getPos(), "Unknown variable name '%s'", lhse->getName().c_str());
    }

    // Implicit casting to destination type when assigning to variables.
    Type *varType = variable->getType()->getContainedType(0);
    val = Helpers::CreateImplicitCast(codegen, val, varType);

    codegen->getBuilder().CreateStore(val, variable);
    return val;
}

Value *AstBinaryOperatorExpr::VariableOpAssignment(CodeGenerator *codegen) {
    TokenType operation;
    std::string operStr;
    switch (this->Operator) {
    default: return Helpers::Error(this->getPos(), "Unknown operator '%s'.", this->OperatorString.c_str());
    case tok_plusequals:            // '+='
        operation = (TokenType)'+';
        operStr = "+";
        break;
    case tok_minusequals:           // '-='
        operation = (TokenType)'-';
        operStr = "-";
        break;
    case tok_multequals:            // '*='
        operation = (TokenType)'*';
        operStr = "*";
        break;
    case tok_divequals:             // '/='
        operation = (TokenType)'/';
        operStr = "/";
        break;
    case tok_modequals:             // '%='
        operation = (TokenType)'%';
        operStr = "%";
        break;
    case tok_andequals:             // '&='
        operation = (TokenType)'&';
        operStr = "&";
        break;
    case tok_orequals:              // '|='
        operation = (TokenType)'|';
        operStr = "|";
        break;
    case tok_xorequals:             // '^='
        operation = (TokenType)'^';
        operStr = "^";
        break;
    case tok_leftshiftequal:        // '<<='
        operation = tok_leftshift;
        operStr = "<<";
        break;
    case tok_rightshiftequal:       // '>>='
        operation = tok_rightshift;
        operStr = ">>";
        break;
    }
    int line = this->LHS->getPos().LineNumber;
    int column = this->LHS->getPos().ColumnNumber;
    // Doing a trick here where we just expand the operation. e.g x += 5 -> x = (x + 5)
    AstBinaryOperatorExpr *eval = new AstBinaryOperatorExpr(operStr, operation, this->LHS, this->RHS, line, column);
    AstBinaryOperatorExpr *assign = new AstBinaryOperatorExpr("=", (TokenType)'=', this->LHS, eval, line, column);
    return assign->Codegen(codegen);
}

Value *AstBinaryOperatorExpr::Codegen(CodeGenerator *codegen) {
    switch (this->Operator) {
    case '=':
        return this->VariableAssignment(codegen);
    case tok_plusequals:            // '+='
    case tok_minusequals:           // '-='
    case tok_multequals:            // '*='
    case tok_divequals:             // '/='
    case tok_modequals:             // '%='
    case tok_andequals:             // '&='
    case tok_orequals:              // '|='
    case tok_xorequals:             // '^='
    case tok_leftshiftequal:        // '<<='
    case tok_rightshiftequal:       // '>>='
        return this->VariableOpAssignment(codegen);
    }
    Value *l = this->LHS->Codegen(codegen);
    Value *r = this->RHS->Codegen(codegen);
    if (l == nullptr || r == nullptr) {
        return Helpers::Error(this->getPos(), "Could not evaluate expression!");
    }
    Type *lType = l->getType();
    Type *rType = r->getType();
    if (lType->isIntegerTy() && rType->isIntegerTy()) {
        Helpers::NormalizeIntegerWidths(codegen, l, r);
    }
    bool isUnsigned = Helpers::IsUnsigned(this->LHS->getNodeType()) && Helpers::IsUnsigned(this->RHS->getNodeType());
    auto funcPtr = Helpers::GetBinopCodeGenFuncPointer(this->Operator, lType, rType, isUnsigned);
    if (funcPtr == nullptr) {
        return Helpers::Error(this->getPos(), "Operator '%s' does not exist for '%s' and '%s'",
            this->OperatorString.c_str(), Helpers::GetLLVMTypeName(lType).c_str(), Helpers::GetLLVMTypeName(rType).c_str());
    }
    return funcPtr(codegen, l, r);
}



