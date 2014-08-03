#include "AstWhileExpr.h"

#include "../CodeGenerator/CodeGenerator.h"
#include "../CodeGenerator/CodeGeneratorHelpers.h"

using namespace llvm;

AstWhileExpr::AstWhileExpr(IAstExpression *condition, const std::vector<IAstExpression*> &whileBody,
    int line, int column)
    : Condition(condition)
    , WhileBody(whileBody) {
    setNodeType(node_while);
    setPos(PossiblePosition{ line, column });
}
AstWhileExpr::~AstWhileExpr() {
    delete Condition;
    while (!WhileBody.empty()) delete WhileBody.back(), WhileBody.pop_back();
}

Value *AstWhileExpr::Codegen(CodeGenerator *codegen) {
    Value *cond = this->Condition->Codegen(codegen);
    if (cond == nullptr)
        return Helpers::Error(this->Condition->getPos(), "Could not evaluate 'while' condition.");
    if (!cond->getType()->isIntegerTy()) // all booleans are integers, so check if the condition is not one and error out.
        return Helpers::Error(this->Condition->getPos(), "'while' condition not boolean type.");

    Value *tobool = Helpers::ToBoolean(codegen, cond);

    Function *func = codegen->getBuilder().GetInsertBlock()->getParent();

    BasicBlock *outsideNestBB = codegen->getMergeBlock(); // save the outside to branch to at end of loop.
    BasicBlock *whileEndBB = BasicBlock::Create(codegen->getContext(), "while_end", func, outsideNestBB); // end of while loop jumps here
    codegen->setMergeBlock(whileEndBB); // set the outside merge block for any nested blocks
    BasicBlock *whileBodyBB = BasicBlock::Create(codegen->getContext(), "while_body", func, whileEndBB);

    // evaluate the condition and branch accordingly, 
    // e.g while (false) {...} should be skipped. 
    codegen->getBuilder().CreateCondBr(tobool, whileBodyBB, whileEndBB);

    // emit the body
    codegen->getBuilder().SetInsertPoint(whileBodyBB);
    bool whileHitReturn = false;
    Helpers::EmitBlock(codegen, this->WhileBody, true, &whileHitReturn); // emit the while block


    if (whileBodyBB->getTerminator() == nullptr)
        codegen->getBuilder().CreateBr(whileEndBB);

    if (whileEndBB->getTerminator() == nullptr) {
        // re-evaluate the condition
        codegen->getBuilder().SetInsertPoint(whileEndBB);
        cond = this->Condition->Codegen(codegen);
        tobool = Helpers::ToBoolean(codegen, cond);
        codegen->getBuilder().CreateCondBr(tobool, whileBodyBB, outsideNestBB); // evaluate the condition and branch accordingly
    }

    whileBodyBB = codegen->getBuilder().GetInsertBlock();
    whileEndBB = codegen->getBuilder().GetInsertBlock();
    codegen->getBuilder().SetInsertPoint(outsideNestBB);
    return whileEndBB;// Helpers::Error(this->getPos(), "While loop not yet implemented");
}