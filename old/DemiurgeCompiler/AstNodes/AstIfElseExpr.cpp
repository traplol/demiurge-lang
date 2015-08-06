#include "AstIfElseExpr.h"

#include "llvm/IR/Function.h"

#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"

using namespace llvm;

AstIfElseExpr::AstIfElseExpr(IAstExpression *condition, const std::vector<IAstExpression*> &ifBody,
    const std::vector<IAstExpression*> &elseBody, int line, int column)
    : Condition(condition)
    , IfBody(ifBody)
    , ElseBody(elseBody) {
    setNodeType(node_ifelse);
    setPos(PossiblePosition{ line, column });
}
AstIfElseExpr::~AstIfElseExpr() {
    delete Condition;
    while (!IfBody.empty()) delete IfBody.back(), IfBody.pop_back();
    while (!ElseBody.empty()) delete ElseBody.back(), ElseBody.pop_back();
}

Value *AstIfElseExpr::Codegen(CodeGenerator *codegen) {
    
    Value *condition = this->Condition->Codegen(codegen);
    if (condition == nullptr) {
        return Helpers::Error(this->Condition->getPos(), "Could not evaludate 'if' condition.");
    }
    if (!condition->getType()->isIntegerTy()) {
        return Helpers::Error(this->Condition->getPos(), "'if' condition not boolean type.");
    }
    // Convert the condition to a boolean, anything not zero is true.
    Value *asBoolean = Helpers::ToBoolean(codegen, condition);
    Function *func = codegen->getCurrentFunction();

    BasicBlock *outsideBB = codegen->getOutsideBlock(); // save the outside block for restoration later
    
    BasicBlock *ifEndBB = BasicBlock::Create(codegen->getContext(), "if_end", func, outsideBB);
    BasicBlock *ifFalseBB = nullptr;
    if (this->ElseBody.size()) {
        ifFalseBB = BasicBlock::Create(codegen->getContext(), "if_false", func, ifEndBB);
    }
    BasicBlock *ifTrueBB = BasicBlock::Create(codegen->getContext(), "if_true", func, ifFalseBB == nullptr ? ifEndBB : ifFalseBB);

    codegen->setOutsideBlock(ifEndBB);
    
    // Conditional branch to true or false branch.
    codegen->getBuilder().CreateCondBr(asBoolean, ifTrueBB, ifFalseBB == nullptr ? ifEndBB : ifFalseBB);

    // Emit the 'true' block.
    codegen->getBuilder().SetInsertPoint(ifTrueBB);
    Helpers::EmitScopeBlock(codegen, this->IfBody, true);
    // if the true block doesn't have a terminator, we need to go to the end block
    if (ifTrueBB->getTerminator() == nullptr) { 
        codegen->getBuilder().CreateBr(ifEndBB);
    }

    if (ifFalseBB != nullptr) {
        // Emit the 'false' block
        codegen->getBuilder().SetInsertPoint(ifFalseBB);
        Helpers::EmitScopeBlock(codegen, this->ElseBody, true);
        // if the false block doesn't have a terminator, we need to go to the end block
        if (ifFalseBB->getTerminator() == nullptr) {
            codegen->getBuilder().CreateBr(ifEndBB);
        }
    }
    
    codegen->getBuilder().SetInsertPoint(ifEndBB);

    ifTrueBB = codegen->getBuilder().GetInsertBlock();
    ifFalseBB = codegen->getBuilder().GetInsertBlock();
    ifEndBB = codegen->getBuilder().GetInsertBlock();
    codegen->setOutsideBlock(outsideBB); // restore the outside block

    return ifEndBB;
}