#ifndef _AST_NODES_H
#define _AST_NODES_H

#include <vector>
#include <string>
#include <map>

#include "PossiblePosition.h"
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
    PossiblePosition Pos;
    tag_AstNodeType NodeType;
    IExpressionAST() : NodeType(node_default){}
    virtual ~IExpressionAST(){}
    virtual llvm::Value *Codegen(CodeGenerator *codegen) = 0;
};

class AstDoubleNode : public IExpressionAST {
    double Value;
public:
    AstDoubleNode(double value, int line, int column)
        : Value(value) {
        NodeType = node_double;
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstIntegerNode : public IExpressionAST {
    // The reason this is unsigned is because rather than trying to parse a negative number
    // we will rely on the negate, '-', operator to create negative numbers.
    unsigned long long int Value;
public:
    AstIntegerNode(unsigned long long int value, int line, int column)
        : Value(value) {
        NodeType = node_integer;
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstBooleanNode : public IExpressionAST {
    bool Value;
public:
    AstBooleanNode(bool value, int line, int column)
        : Value(value) {
        NodeType = node_boolean;
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstStringNode : public IExpressionAST {
    std::string Value;
public:
    AstStringNode(const std::string &value, int line, int column)
        : Value(value) {
        NodeType = node_string;
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstBinaryOperatorExpr : public IExpressionAST {
    std::string OperatorString;
    TokenType Operator;
    IExpressionAST *LHS, *RHS;
public:
    AstBinaryOperatorExpr(const std::string &operStr, TokenType oper, IExpressionAST *lhs, 
        IExpressionAST *rhs, int line, int column)
        : OperatorString(operStr)
        , Operator(oper)
        , LHS(lhs)
        , RHS(rhs) {
        NodeType = node_binary_operation;
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
    ~AstBinaryOperatorExpr() {
        delete LHS, RHS;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstReturnNode : public IExpressionAST {
    IExpressionAST *Expr;
public:
    AstReturnNode(IExpressionAST *expr, int line, int column)
        : Expr(expr) {
        NodeType = node_return;
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
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
    AstIfElseExpression(IExpressionAST *condition, const std::vector<IExpressionAST*> &ifBody,
        const std::vector<IExpressionAST*> &elseBody, int line, int column)
        : Condition(condition)
        , IfBody(ifBody)
        , ElseBody(elseBody) {
        NodeType = node_ifelse;
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
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
    AstWhileExpression(IExpressionAST *condition, const std::vector<IExpressionAST*> &whileBody,
        int line, int column)
        : Condition(condition)
        , WhileBody(whileBody) {
        NodeType = node_while;
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
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
    AstCallExpression(const std::string &name, const std::vector<IExpressionAST*> &args, int line, int column)
        : Name(name)
        , Args(args) {
        NodeType = node_call;
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
    ~AstCallExpression() {
        while (!Args.empty()) delete Args.back(), Args.pop_back();
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstTypeNode {
public:
    PossiblePosition Pos;
    AstNodeType TypeType;
    AstTypeNode(AstNodeType type, int line, int column)
        : TypeType(type) {
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
    llvm::Type *GetLLVMType(CodeGenerator *codegen);
};

class AstVariableNode : public IExpressionAST {
    std::string Name;
public:
    AstVariableNode(const std::string &name, int line, int column)
        : Name(name) {
        NodeType = node_variable;
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
};

class AstVarNode : public IExpressionAST {
    std::string Name;
    AstTypeNode *InferredType;
    IExpressionAST *AssignmentExpression;
public:
    AstVarNode(const std::string &name, IExpressionAST *assignmentExpression, AstTypeNode *inferredType,
        int line, int column)
        : Name(name)
        , AssignmentExpression(assignmentExpression)
        , InferredType(inferredType) {
        NodeType = node_var;
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
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
    PossiblePosition Pos;
    std::string Name;
    std::map<std::string, AstTypeNode*> Args;
    AstTypeNode *ReturnType;
    PrototypeAst(const std::string &name, AstTypeNode *returnType, const std::map<std::string, AstTypeNode*> &args,
        int line, int column)
        : Name(name)
        , ReturnType(returnType)
        , Args(args) {
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
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
    PossiblePosition Pos;
    PrototypeAst *Prototype;
    std::vector<IExpressionAST*> FunctionBody;
    FunctionAst(PrototypeAst *prototype, const std::vector<IExpressionAST*> &functionBody, int line, int column)
        : Prototype(prototype)
        , FunctionBody(functionBody) {
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
    ~FunctionAst() {
        delete Prototype;
        while (!FunctionBody.empty()) delete FunctionBody.back(), FunctionBody.pop_back();
    }
    virtual llvm::Function *Codegen(CodeGenerator *codegen);
};


#endif