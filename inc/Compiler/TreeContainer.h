#ifndef _TREE_CONTAINER_H
#define _TREE_CONTAINER_H

#include <vector>

class ClassAst;
class PrototypeAst;
class FunctionAst;
class IAstExpression;

struct TreeContainer {
    ~TreeContainer();
    std::vector<PrototypeAst*> ExternalDeclarations;
    std::vector<IAstExpression*> TopLevelExpressions;
    std::vector<FunctionAst*> FunctionDefinitions;
    std::vector<ClassAst*> ClassDefinitions;
};

#endif