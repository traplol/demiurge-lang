#include "TreeContainer.h"

TreeContainer::~TreeContainer() {
    while (!ClassDefinitions.empty()) delete ClassDefinitions.back(), ClassDefinitions.pop_back();
    while (!FunctionDefinitions.empty()) delete FunctionDefinitions.back(), FunctionDefinitions.pop_back();
    while (!TopLevelExpressions.empty()) delete TopLevelExpressions.back(), TopLevelExpressions.pop_back();
    while (!ExternalDeclarations.empty()) delete ExternalDeclarations.back(), ExternalDeclarations.pop_back();
}