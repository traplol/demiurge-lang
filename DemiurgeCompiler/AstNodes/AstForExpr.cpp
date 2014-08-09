#include "AstForExpr.h"

using namespace llvm;
AstForExpr::AstForExpr(IAstExpression *condition, const std::vector<IAstExpression*> &init,
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
    return nullptr;
}
