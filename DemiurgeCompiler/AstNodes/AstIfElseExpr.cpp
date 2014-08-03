#include "AstIfElseExpr.h"

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
    Value *cond = this->Condition->Codegen(codegen);
    if (cond == nullptr)
        return Helpers::Error(this->Condition->getPos(), "Could not evaluate 'if' condition.");
    if (!cond->getType()->isIntegerTy()) // all booleans are integers, so check if the condition is not one and error out.
        return Helpers::Error(this->Condition->getPos(), "'if' condition not boolean type.");
    Value *tobool = Helpers::ToBoolean(codegen, cond);
    Function *func = codegen->getBuilder().GetInsertBlock()->getParent();

    // Create blocks for the 'if' cases.
    BasicBlock *outsideNestBB = codegen->getMergeBlock(); // save the outside to branch to at end of loop.
    BasicBlock *ifendBB = BasicBlock::Create(codegen->getContext(), "if_end", func, outsideNestBB); // both blocks merge back here.
    codegen->setMergeBlock(ifendBB);
    BasicBlock *ifelseBB = BasicBlock::Create(codegen->getContext(), "if_false", func, ifendBB);
    BasicBlock *ifthenBB = BasicBlock::Create(codegen->getContext(), "if_true", func, ifelseBB);


    // Create the conditional branch.
    codegen->getBuilder().CreateCondBr(tobool, ifthenBB, ifelseBB);
    bool goesToMerge = false;

    // emit 'if.then' block.
    codegen->getBuilder().SetInsertPoint(ifthenBB);
    bool ifthenReturns;
    Helpers::EmitBlock(codegen, this->IfBody, true, &ifthenReturns);

    if (ifthenBB->getTerminator() == nullptr) { // if there was no return statement within the else block
        codegen->getBuilder().CreateBr(ifendBB); // go to merge since there was no return in the then block
        goesToMerge = true;
    }


    // emit 'if.else' block
    codegen->getBuilder().SetInsertPoint(ifelseBB);
    bool ifelseReturns;
    Helpers::EmitBlock(codegen, this->ElseBody, true, &ifelseReturns);

    if (ifelseBB->getTerminator() == nullptr) { // if there was no return statement within the else block
        codegen->getBuilder().CreateBr(ifendBB); // go to merge since there was no return in the else block
        goesToMerge = true;
    }

    if (ifelseReturns && ifthenReturns) {
        goesToMerge = false;
        //ifendBB->removeFromParent();
    }
    if (ifendBB->getTerminator() != nullptr) {
        goesToMerge = false;
    }
    if (goesToMerge || ifendBB->getTerminator() == nullptr) {// merge back to the rest of the code.
        codegen->getBuilder().SetInsertPoint(ifendBB);
        codegen->getBuilder().CreateBr(outsideNestBB);
    }
    ifthenBB = codegen->getBuilder().GetInsertBlock();
    ifelseBB = codegen->getBuilder().GetInsertBlock();
    ifendBB = codegen->getBuilder().GetInsertBlock();
    codegen->getBuilder().SetInsertPoint(outsideNestBB);
    return ifendBB;
}