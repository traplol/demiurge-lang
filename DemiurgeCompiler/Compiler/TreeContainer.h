#ifndef _TREE_CONTAINER_H
#define _TREE_CONTAINER_H

#include <vector>
#include "../AstNodes/PrototypeAst.h"
#include "../AstNodes/FunctionAst.h"
#include "../AstNodes/IAstExpression.h"

struct TreeContainer {
    std::vector<PrototypeAst*> ExternalDeclarations;
    std::vector<IAstExpression*> TopLevelExpressions;
    std::vector<FunctionAst*> FunctionDefinitions;
};

#endif