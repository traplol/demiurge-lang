#ifndef _AST_NODES_H
#define _AST_NODES_H

#include <vector>
#include <string>
#include <map>

#include "AstNodeTypes.h"
#include "TokenTypes.h"

class CodeGenerator;
namespace llvm {
    class Value;
    class Type;
    class Function;
}

class IExpressionAST {
public:
    tag_AstNodeType NodeType;
    IExpressionAST() : NodeType(node_default){}
    virtual ~IExpressionAST(){}
    virtual llvm::Value *Codegen(CodeGenerator *codegen) = 0;
};

class AstDoubleNode : public IExpressionAST {
    double Value;
public:
    AstDoubleNode(double value)
        : Value(value) {
        NodeType = node_double;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstIntegerNode : public IExpressionAST {
    // The reason this is unsigned is because rather than trying to parse a negative number
    // we will rely on the negate, '-', operator to create negative numbers.
    unsigned long long int Value;
public:
    AstIntegerNode(unsigned long long int value)
        : Value(value) {
        NodeType = node_integer;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstBooleanNode : public IExpressionAST {
    bool Value;
public:
    AstBooleanNode(bool value)
        : Value(value) {
        NodeType = node_boolean;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstStringNode : public IExpressionAST {
    std::string Value;
public:
    AstStringNode(const std::string &value)
        : Value(value) {
       NodeType = node_string;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstBinaryOperatorExpr : public IExpressionAST {
    std::string OperatorString;
    TokenType Operator;
    IExpressionAST *LHS, *RHS;
public:
    AstBinaryOperatorExpr(const std::string &operStr, TokenType oper, IExpressionAST *lhs, IExpressionAST *rhs)
        : OperatorString(operStr)
        , Operator(oper)
        , LHS(lhs)
        , RHS(rhs) {
       NodeType = node_binary_operation;
    }
    ~AstBinaryOperatorExpr() {
        delete LHS, RHS;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstReturnNode : public IExpressionAST {
    IExpressionAST *Expr;
public:
    AstReturnNode(IExpressionAST *expr)
        : Expr(expr) {
       NodeType = node_return;
    }
    ~AstReturnNode() {
        delete Expr;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstIfElseExpression : public IExpressionAST {
    IExpressionAST *Condition;
    std::vector<IExpressionAST*> IfBody, ElseBody;
public:
    AstIfElseExpression(IExpressionAST *condition, const std::vector<IExpressionAST*> &ifBody, const std::vector<IExpressionAST*> &elseBody)
        : Condition(condition)
        , IfBody(ifBody)
        , ElseBody(elseBody) {
       NodeType = node_ifelse;
    }
    ~AstIfElseExpression() {
        delete Condition;
        while (!IfBody.empty()) delete IfBody.back(), IfBody.pop_back();
        while (!ElseBody.empty()) delete ElseBody.back(), ElseBody.pop_back();
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstWhileExpression : public IExpressionAST {
    IExpressionAST *Condition;
    std::vector<IExpressionAST*> WhileBody;
public:
    AstWhileExpression(IExpressionAST *condition, const std::vector<IExpressionAST*> &whileBody)
        : Condition(condition)
        , WhileBody(whileBody) {
       NodeType = node_while;
    }
    ~AstWhileExpression() {
        delete Condition;
        while (!WhileBody.empty()) delete WhileBody.back(), WhileBody.pop_back();
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstCallExpression : public IExpressionAST {
    std::string Name;
    std::vector<IExpressionAST*> Args;
public:
    AstCallExpression(const std::string &name, const std::vector<IExpressionAST*> &args)
        : Name(name)
        , Args(args) {
       NodeType = node_call;
    }
    ~AstCallExpression() {
        while (!Args.empty()) delete Args.back(), Args.pop_back();
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstTypeNode {
public:
    AstNodeType TypeType;
    AstTypeNode(AstNodeType type)
        : TypeType(type) {}
    llvm::Type *GetLLVMType(CodeGenerator *codegen);
};

class AstVariableNode : public IExpressionAST {
    std::string Name;
public:
    AstVariableNode(const std::string &name)
        : Name(name) {
       NodeType = node_variable;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstVarNode : public IExpressionAST {
    std::string Name;
    AstTypeNode *InferredType;
    IExpressionAST *AssignmentExpression;
public:
    AstVarNode(const std::string &name, IExpressionAST *assignmentExpression, AstTypeNode *inferredType)
        : Name(name)
        , AssignmentExpression(assignmentExpression)
        , InferredType(inferredType) {
       NodeType = node_var;
    }
    ~AstVarNode() {
        delete InferredType, AssignmentExpression;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstTopLevelExpression : public IExpressionAST {

};

class PrototypeAst {
public:
    std::string Name;
    std::map<std::string, AstTypeNode*> Args;
    AstTypeNode *ReturnType;
    PrototypeAst(const std::string &name, AstTypeNode *returnType, const std::map<std::string, AstTypeNode*> &args)
        : Name(name)
        , ReturnType(returnType)
        , Args(args) {}
    ~PrototypeAst() {
        delete ReturnType;
        for (auto begin = Args.begin(); begin != Args.end(); ++begin) {
            delete begin->second;
        }
    }
    virtual llvm::Function *Codegen(CodeGenerator *codegen);
    void CreateArgumentAllocas(CodeGenerator *codegen, llvm::Function *func);
};

class FunctionAst {
public:
    PrototypeAst *Prototype;
    std::vector<IExpressionAST*> FunctionBody;
    FunctionAst(PrototypeAst *prototype, const std::vector<IExpressionAST*> &functionBody)
        : Prototype(prototype)
        , FunctionBody(functionBody) {}
    ~FunctionAst() {
        delete Prototype;
        while (!FunctionBody.empty()) delete FunctionBody.back(), FunctionBody.pop_back();
    }
    virtual llvm::Function *Codegen(CodeGenerator *codegen);
};


#endif