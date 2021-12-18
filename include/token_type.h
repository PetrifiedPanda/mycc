#ifndef TOKEN_TYPE_H
#define TOKEN_TYPE_H

// TODO: update with new grammar

#include <stdbool.h>

enum token_type {
    IDENTIFIER,
    I_CONSTANT,
    F_CONSTANT,
    STRING_LITERAL,
    FUNC_NAME, // __func__

    SIZEOF,
    PTR_OP, // ->
    INC_OP,
    DEC_OP,
    LEFT_OP, // <<
    RIGHT_OP, // >>
    LE_OP,
    GE_OP,
    EQ_OP,
    NE_OP,
    AND_OP, // &&
    OR_OP, // ||
    MUL_ASSIGN,
    DIV_ASSIGN,
    MOD_ASSIGN,
    ADD_ASSIGN,
    SUB_ASSIGN,
    LEFT_ASSIGN,
    RIGHT_ASSIGN,
    AND_ASSIGN,
    OR_ASSIGN,
    XOR_ASSIGN,
    TYPE_NAME,
    TYPEDEF,
    EXTERN,
    STATIC,
    AUTO,
    REGISTER,
    INLINE,

    BOOL, // _Bool
    CHAR,
    SHORT,
    INT,
    LONG,
    SIGNED,
    UNSIGNED,
    FLOAT,
    DOUBLE,
    VOID,

    COMPLEX, // _Complex
    IMAGINARY, // _Imaginary

    CONST,
    VOLATILE,
    RESTRICT,
    ATOMIC,

    STRUCT,
    UNION,
    ENUM,
    ELLIPSIS, // ...
    CASE,
    DEFAULT,
    IF,
    ELSE,
    SWITCH,
    WHILE,
    DO,
    FOR,
    GOTO,
    CONTINUE,
    BREAK,
    RETURN,

    ALIGNAS, // _Alignas
    ALIGNOF, // _Alignof
    GENERIC, // _Generic
    NORETURN, // _Noreturn
    STATIC_ASSERT, // _Static_assert
    THREAD_LOCAL, // _Thread_local

    SEMICOLON,
    LBRACKET, // (
    RBRACKET, // )
    LBRACE, // {
    RBRACE, // }
    LINDEX, // [
    RINDEX, // ]
    DOT,
    AND, // &
    OR, // |
    XOR, // ^
    NOT, // !
    BNOT, // ~
    SUB,
    ADD,
    ASTERISK,
    DIV,
    MOD,
    LT, // <
    GT, // >
    QMARK, // ?
    COLON, // :
    ASSIGN, // =
    COMMA,
    INVALID
};

/**
 * @brief Gets a spelling for the given token_type
 * 
 * @param type Type to get the spelling for
 * @return const char* The spelling of the given token type, if it is unambiguous, otherwise NULL
 */
const char* get_spelling(enum token_type type);

/**
 * @brief Gets a string to identify the token_type
 * 
 * @param type enum token_type value
 * @return const char* A string that is identical to the spelling of the enum value
 */
const char* get_type_str(enum token_type type);

bool is_unary_op(enum token_type t);
bool is_assign_op(enum token_type t);
bool is_storage_class_spec(enum token_type t);
bool is_keyword_type_spec(enum token_type t);
bool is_type_qual(enum token_type t);

bool is_shift_op(enum token_type t);
bool is_rel_op(enum token_type t);
bool is_mul_op(enum token_type t);
bool is_add_op(enum token_type t);
bool is_eq_op(enum token_type t);

#endif

