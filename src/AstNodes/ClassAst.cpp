#include "AstNodes/ClassAst.h"

#include "AstNodes/FunctionAst.h"
#include "AstNodes/AstVarExpr.h"

ClassAst::ClassAst(){}
ClassAst::ClassAst(const std::string &name, FunctionAst *ctor, const std::vector<AstVarExpr*> &publicFields, const std::vector<AstVarExpr*> &privateFields,
    const std::vector<FunctionAst*> &publicFunctions, const std::vector<FunctionAst*> &privateFunctions)
    : Name(name)
    , Constructor(ctor)
    , PublicFields(publicFields)
    , PrivateFields(privateFields)
    , PublicFunctions(publicFunctions)
    , PrivateFunctions(privateFunctions){}

ClassAst::~ClassAst() {
    delete Constructor;
    while (!PublicFields.empty()) delete PublicFields.back(), PublicFields.pop_back();
    while (!PrivateFields.empty()) delete PrivateFields.back(), PrivateFields.pop_back();
    while (!PublicFunctions.empty()) delete PublicFunctions.back(), PublicFunctions.pop_back();
    while (!PrivateFunctions.empty()) delete PrivateFunctions.back(), PrivateFunctions.pop_back();
}

void ClassAst::setName(const std::string &name) {
    Name = name;
}
std::string ClassAst::getName() const {
    return Name;
}
bool ClassAst::setConstructor(FunctionAst *ctor) {
    if (Constructor != nullptr || ctor == nullptr) {
        return false;
    }
    Constructor = ctor;
    return true;
}
bool ClassAst::pushPublicField(AstVarExpr *publicField) {
    if (publicField == nullptr) {
        return false;
    }
    PublicFields.push_back(publicField);
    return true;
}
bool ClassAst::pushPrivateField(AstVarExpr *privateField) {
    if (privateField == nullptr) {
        return false;
    }
    PrivateFields.push_back(privateField);
    return true;
}
bool ClassAst::pushPublicFunction(FunctionAst *publicFunction) {
    if (publicFunction == nullptr) {
        return false;
    }
    PublicFunctions.push_back(publicFunction);
    return true;
}
bool ClassAst::pushPrivateFunction(FunctionAst *privateFunction) {
    if (privateFunction == nullptr) {
        return false;
    }
    PrivateFunctions.push_back(privateFunction);
    return true;
}
