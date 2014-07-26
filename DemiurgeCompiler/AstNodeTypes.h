#ifndef _AST_NODE_TYPES_H
#define _AST_NODE_TYPES_H

typedef enum tag_AstNodeType {
    node_default,
    node_function,
    node_prototype,
    node_binary_operation,
    node_call,
    node_variable,
    node_var,
    node_return,
    node_ifelse,
    node_while,
    node_for,
    node_boolean,
    node_double,
    node_integer,
    node_string,
    node_void,
    node_toplevel,
    node_type,
} AstNodeType;

#endif