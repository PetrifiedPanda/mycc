#ifndef TOKEN_TYPE_H
#define TOKEN_TYPE_H

#include <stdbool.h>

enum token_kind {
    IDENTIFIER,
    I_CONSTANT,
    F_CONSTANT,
    STRING_LITERAL,
    TYPEDEF_NAME,

    // Keywords
    KEYWORDS_START,
    FUNC_NAME = KEYWORDS_START, // __func__
    SIZEOF,
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
    COMPLEX,   // _Complex
    IMAGINARY, // _Imaginary
    CONST,
    VOLATILE,
    RESTRICT,
    ATOMIC,
    STRUCT,
    UNION,
    ENUM,
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
    ALIGNAS,       // _Alignas
    ALIGNOF,       // _Alignof
    GENERIC,       // _Generic
    NORETURN,      // _Noreturn
    STATIC_ASSERT, // _Static_assert
    THREAD_LOCAL,  // _Thread_local
    KEYWORDS_END,

    // Punctuation
    SEMICOLON = KEYWORDS_END,
    LBRACKET, // (
    RBRACKET, // )
    LBRACE,   // {
    RBRACE,   // }
    LINDEX,   // [
    RINDEX,   // ]
    DOT,
    AND,  // &
    OR,   // |
    XOR,  // ^
    NOT,  // !
    BNOT, // ~
    SUB,
    ADD,
    ASTERISK,
    DIV,
    MOD,
    LT,     // <
    GT,     // >
    QMARK,  // ?
    COLON,  // :
    ASSIGN, // =
    COMMA,
    PTR_OP, // ->
    INC_OP,
    DEC_OP,
    LEFT_OP,  // <<
    RIGHT_OP, // >>
    LE_OP,
    GE_OP,
    EQ_OP,
    NE_OP,
    AND_OP, // &&
    OR_OP,  // ||
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
    ELLIPSIS, // ...

    // Preprocessor only
    STRINGIFY_OP, // #
    CONCAT_OP,    // ##

    INVALID
};

/**
 * @brief Gets a spelling for the given token_kind
 *
 * @param kind Type to get the spelling for
 * @return const char* The spelling of the given token kind, if it is
 * unambiguous, otherwise NULL
 */
const char* get_token_kind_spelling(enum token_kind kind);

/**
 * @brief Gets a string to identify the token_kind
 * 
 * @return const char* A string that is identical to the spelling of the enum
 * value
 */
const char* get_token_kind_str(enum token_kind kind);

#endif

