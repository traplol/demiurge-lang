#include "AstNodes/AstForExpr.h"
#include "CodeGenerator/CodeGenerator.h"
#include "CodeGenerator/CodeGeneratorHelpers.h"
#include "llvm/IR/Function.h"

using namespace llvm;
AstForExpr::AstForExpr(const std::vector<IAstExpression*> &init, IAstExpression *condition,
    const std::vector<IAstExpression*> &afterThought, const std::vector<IAstExpression*> &body, 
    int line, int column)
    : Condition(condition)
    , Init(init)
    , Afterthought(afterThought)
    , Body(body) {
    setNodeType(node_for);
    setPos(PossiblePosition{ line, column });
}
AstForExpr::~AstForExpr() {
    delete Condition;
    while (!Init.empty()) delete Init.back(), Init.pop_back();
    while (!Afterthought.empty()) delete Afterthought.back(), Afterthought.pop_back();
}
Value *AstForExpr::Codegen(CodeGenerator *codegen) {
    auto x = this;

    Function *func = codegen->getCurrentFunction();
    BasicBlock *outsideBB = codegen->getOutsideBlock();
    BasicBlock *loopEndBB = BasicBlock::Create(codegen->getContext(), "loop_end", func, outsideBB);
    BasicBlock *loopBodyBB = BasicBlock::Create(codegen->getContext(), "loop_body", func, loopEndBB);
    codegen->setOutsideBlock(loopEndBB);

    unsigned varCountBeforeInit = codegen->getVarCount(); // Keep track of the variables created within the for loop init.
    // Emit the init.
    for (unsigned i = 0, size = this->Init.size(); i < size; ++i) {
        IAstExpression *expr = this->Init[i];
        if (expr == nullptr || expr->Codegen(codegen) == nullptr) {
            return Helpers::Error(this->getPos(), "Could not initialize 'for' loop.");
        }
    }
    unsigned varCountAfterInit = codegen->getVarCount();

    if (this->Condition == nullptr) { // Disallow for loop without condition.
        return Helpers::Error(this->getPos(), "For loop must have a condition.");
    }
    Value *cond = this->Condition->Codegen(codegen);
    if (!cond->getType()->isIntegerTy()) { // Condition isn't a possible boolean type.
        return Helpers::Error(this->Condition->getPos(), "'for' condition not boolean");
    }
    Value *toBool = Helpers::ToBoolean(codegen, cond);
    // Eval the bool and branch accordingly
    codegen->getBuilder().CreateCondBr(toBool, loopBodyBB, loopEndBB);

    // Emit the loop body.
    codegen->getBuilder().SetInsertPoint(loopBodyBB);
    Helpers::EmitScopeBlock(codegen, this->Body, true);
    // Emit the afterthough
    Helpers::EmitScopeBlock(codegen, this->Afterthought, true);
    // Eval the condition, branch accordingly
    cond = this->Condition->Codegen(codegen);
    toBool = Helpers::ToBoolean(codegen, cond);
    codegen->getBuilder().CreateCondBr(toBool, loopBodyBB, loopEndBB);

    // Cleanup the variables created in the init.
    codegen->popFromScopeStack(varCountAfterInit - varCountBeforeInit);

    // Set insert point to the loop and and cleanup.
    codegen->getBuilder().SetInsertPoint(loopEndBB);
    loopBodyBB = codegen->getBuilder().GetInsertBlock();
    loopEndBB = codegen->getBuilder().GetInsertBlock();
    codegen->setOutsideBlock(outsideBB);
    return loopEndBB;
}
