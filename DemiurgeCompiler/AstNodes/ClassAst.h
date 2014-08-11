#ifndef _CLASS_AST_H
#define _CLASS_AST_H

#include "AST_DEPENDENCIES.h"
//#include "FunctionAst.h"
//#include "AstVarExpr.h"
#include <vector>
#include <string>

class FunctionAst;
class AstVarExpr;

class ClassAst {
    FunctionAst *Constructor;
    std::vector<AstVarExpr*> PublicFields, PrivateFields;
    std::vector<FunctionAst*> PublicFunctions, PrivateFunctions;
    std::string Name;

public:
    ClassAst();
    ClassAst(const std::string &name, FunctionAst *ctor, const std::vector<AstVarExpr*> &publicFields, const std::vector<AstVarExpr*> &privateFields,
        const std::vector<FunctionAst*> &publicFunctions, const std::vector<FunctionAst*> &privateFunctions);
    ~ClassAst();

    void setName(const std::string &name);
    std::string getName() const;

    bool setConstructor(FunctionAst *ctor);

    bool pushPublicField(AstVarExpr *publicField);
    bool pushPrivateField(AstVarExpr *privateField);

    bool pushPublicFunction(FunctionAst *publicFunction);
    bool pushPrivateFunction(FunctionAst *privateFunction);

};

#endif