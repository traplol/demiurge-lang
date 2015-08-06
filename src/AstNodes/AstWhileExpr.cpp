#include "AstNodes/AstWhileExpr.h"

#include "CodeGenerator/CodeGenerator.h"
#include "CodeGenerator/CodeGeneratorHelpers.h"

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
    if (cond == nullptr) {
        return Helpers::Error(this->Condition->getPos(), "Could not evaluate 'while' condition.");
    }
    if (!cond->getType()->isIntegerTy()) {// all booleans are integers, so check if the condition is not one and error out.
        return Helpers::Error(this->Condition->getPos(), "'while' condition not boolean type.");
    }

    Value *tobool = Helpers::ToBoolean(codegen, cond);

    Function *func = codegen->getCurrentFunction();

    BasicBlock *outsideNestBB = codegen->getOutsideBlock(); // save the outside to branch to at end of loop.
    BasicBlock *whileEndBB = BasicBlock::Create(codegen->getContext(), "while_end", func, outsideNestBB); // end of while loop jumps here
    
    BasicBlock *whileBodyBB = BasicBlock::Create(codegen->getContext(), "while_body", func, whileEndBB);

    codegen->setOutsideBlock(whileEndBB);

    // evaluate the condition and branch accordingly, 
    codegen->getBuilder().CreateCondBr(tobool, whileBodyBB, whileEndBB);

    // emit the body
    codegen->getBuilder().SetInsertPoint(whileBodyBB);
    Helpers::EmitScopeBlock(codegen, this->WhileBody, true); // emit the while block
    if (whileBodyBB->getTerminator() == nullptr) {
        // Check our condition
        cond = this->Condition->Codegen(codegen);
        tobool = Helpers::ToBoolean(codegen, cond);
        codegen->getBuilder().CreateCondBr(cond, whileBodyBB, whileEndBB);
    }


    codegen->getBuilder().SetInsertPoint(whileEndBB);

    whileBodyBB = codegen->getBuilder().GetInsertBlock();
    whileEndBB = codegen->getBuilder().GetInsertBlock();
    codegen->setOutsideBlock(outsideNestBB);
    
    return whileEndBB;
}
