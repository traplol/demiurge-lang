#ifndef _AST_NODE_TYPES_H
#define _AST_NODE_TYPES_H

typedef enum tag_AstNodeType {
    node_default,
    node_function,
    node_prototype,
    node_binary_operation,
    node_unary_operation_pre,
    node_unary_operation_post,
    node_call,
    node_variable,
    node_var,
    node_return,
    node_ifelse,
    node_while,
    node_for,
    node_boolean,
    node_double,
    node_float,
    node_signed_integer8,
    node_signed_integer16,
    node_signed_integer32,
    node_signed_integer64,
    node_unsigned_integer8,
    node_unsigned_integer16,
    node_unsigned_integer32,
    node_unsigned_integer64,
    node_string,
    node_void,
    node_struct,
    node_toplevel,
    node_type,
} AstNodeType;

#endif