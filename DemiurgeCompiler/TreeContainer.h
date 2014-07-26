#ifndef _TREE_CONTAINER_H
#define _TREE_CONTAINER_H

#include <vector>
#include "AstNodes.h"

struct TreeContainer {
    std::vector<IExpressionAST*> TopLevelExpressions;
    std::vector<FunctionAst*> FunctionDefinitions;
};

#endif