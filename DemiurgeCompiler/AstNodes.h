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
    PossiblePosition Pos;
    AstNodeType NodeType;
public:
    IExpressionAST() : NodeType(node_default){}
    virtual ~IExpressionAST(){}
    virtual llvm::Value *Codegen(CodeGenerator *codegen) = 0;
    void setNodeType(AstNodeType type) { NodeType = type; }
    void setPos(PossiblePosition pos) { Pos = pos; }
    AstNodeType getNodeType() const { return NodeType; }
    PossiblePosition getPos() const { return Pos; }
};

class AstDoubleNode : public IExpressionAST {
    double Value;
public:
    AstDoubleNode(double value, int line, int column)
        : Value(value) {
        setNodeType(node_double);
        setPos(PossiblePosition{ line, column });
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
        setNodeType(node_integer);
        setPos(PossiblePosition{ line, column });
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    unsigned long long int getValue() const { return Value; }
};

class AstBooleanNode : public IExpressionAST {
    bool Value;
public:
    AstBooleanNode(bool value, int line, int column)
        : Value(value) {
        setNodeType(node_boolean);
        setPos(PossiblePosition{ line, column });
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    bool getValue() const { return Value; }
};

class AstStringNode : public IExpressionAST {
    std::string Value;
public:
    AstStringNode(const std::string &value, int line, int column)
        : Value(value) {
        setNodeType(node_string);
        setPos(PossiblePosition{ line, column });
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    const std::string &getValue() const { return Value; }
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
        setNodeType(node_binary_operation);
        setPos(PossiblePosition{ line, column });
    }
    ~AstBinaryOperatorExpr() {
        delete LHS, RHS;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    virtual llvm::Value *VariableAssignment(CodeGenerator *codegen);
};

class AstReturnNode : public IExpressionAST {
    IExpressionAST *Expr;
public:
    AstReturnNode(IExpressionAST *expr, int line, int column)
        : Expr(expr) {
        setNodeType(node_return);
        setPos(PossiblePosition{ line, column });
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
        setNodeType(node_ifelse);
        setPos(PossiblePosition{ line, column });
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
        setNodeType(node_while);
        setPos(PossiblePosition{ line, column });
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
        setNodeType(node_call);
        setPos(PossiblePosition{ line, column });
    }
    ~AstCallExpression() {
        while (!Args.empty()) delete Args.back(), Args.pop_back();
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    const std::string &getName() const { return Name; }
};

class AstTypeNode {
    PossiblePosition Pos;
    AstNodeType TypeType;
public:
    AstTypeNode(AstNodeType type, int line, int column)
        : TypeType(type) {
        Pos.LineNumber = line;
        Pos.ColumnNumber = column;
    }
    llvm::Type *GetLLVMType(CodeGenerator *codegen);
    PossiblePosition getPos() const { return Pos; }
    AstNodeType getTypeType() const { return TypeType; }
};

class AstVariableNode : public IExpressionAST {
    std::string Name;
public:
    AstVariableNode(const std::string &name, int line, int column)
        : Name(name) {
        setNodeType(node_variable);
        setPos(PossiblePosition{ line, column });
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    const std::string &getName() const { return Name; }
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
        setNodeType(node_var);
        setPos(PossiblePosition{ line, column });
    }
    ~AstVarNode() {
        delete InferredType, AssignmentExpression;
    }
    virtual llvm::Value *Codegen(CodeGenerator *codegen);
    const std::string &getName() const { return Name; }
    AstTypeNode *getInferredType() const { return InferredType; }
};

class AstTopLevelExpression : public IExpressionAST {

};

class PrototypeAst {
    PossiblePosition Pos;
    std::string Name;
    std::map<std::string, AstTypeNode*> Args;
    AstTypeNode *ReturnType;
public:
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

    PossiblePosition getPos() const { return Pos; }
    const std::string &getName() const { return Name; }
    AstTypeNode *getReturnType() const { return ReturnType; }
};

class FunctionAst {
    PossiblePosition Pos;
    PrototypeAst *Prototype;
    std::vector<IExpressionAST*> FunctionBody;
public:
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
    PossiblePosition getPos() const { return Pos; }
    PrototypeAst *getReturnType() const { return Prototype; }
};


#endif