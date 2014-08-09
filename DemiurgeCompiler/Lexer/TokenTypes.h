#ifndef _TOKEN_TYPES_H
#define _TOKEN_TYPES_H

typedef enum tag_TokenType {
    // The integers [0, 255] are reserved for literal ASCII character values.
    
    tok_special_eol = 256,  // special end-of-line token used for parsing in interactive mode.
    tok_identifier,         // an identifier such as 'a'

    tok_var,                // 'var'
    tok_func,               // 'func'
    tok_return,             // 'return'
    tok_extern,             // 'extern'
    tok_if,                 // 'if'
    tok_else,               // 'else'
    tok_while,              // 'while'
    tok_for,                // 'for'
    tok_new,                // 'new'
    
    tok_typeint8,           // 'char', 'int8'
    tok_typeint16,          // 'short', 'int16'
    tok_typeint32,          // 'int', 'int32'
    tok_typeint64,          // 'long', 'int64'
    tok_typeuint8,          // 'uchar', 'uint8'
    tok_typeuint16,         // 'ushort', 'int16'
    tok_typeuint32,         // 'uint', 'uint32'
    tok_typeuint64,         // 'ulong', 'uint64'
    tok_typestring,         // 'string'
    tok_typedouble,         // 'double'
    tok_typefloat,          // 'float'
    tok_typebool,           // 'bool'
    tok_typevoid,           // 'void'

    tok_number,             // number literal such as '42'
    tok_bool,               // boolean literal, either 'true' or 'false'
    tok_string,             // string literal such as "hello world"

    tok_dotdot,             // '..'
    tok_plusplus,           // '++'
    tok_minusminus,         // '--'
    tok_plusequals,         // '+='
    tok_minusequals,        // '-='
    tok_multequals,         // '*='
    tok_divequals,          // '/='
    tok_modequals,          // '%='
    tok_andequals,          // '&='
    tok_orequals,           // '|='
    tok_xorequals,          // '^='
    tok_booleanor,          // '||'
    tok_booleanand,         // '&&'
    tok_lessequal,          // '<='
    tok_greatequal,         // '>='
    tok_equalequal,         // '=='
    tok_notequal,           // '!='
    tok_leftshift,          // '<<'
    tok_rightshift,         // '>>'
    tok_LRSqBrackets,       // '[]'

    tok_dotdotdot,          // '...'
    tok_leftshiftequal,     // '<<='
    tok_rightshiftequal,    // '>>='
} TokenType;

#endif