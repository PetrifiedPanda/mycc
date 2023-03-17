#ifndef TOKEN_TYPE_H
#define TOKEN_TYPE_H

#include <stdbool.h>

enum token_kind {
    TOKEN_IDENTIFIER,
    TOKEN_I_CONSTANT,
    TOKEN_F_CONSTANT,
    TOKEN_STRING_LITERAL,
    TOKEN_TYPEDEF_NAME,

    // Keywords
    TOKEN_KEYWORDS_START,
    TOKEN_FUNC_NAME = TOKEN_KEYWORDS_START, // __func__
    TOKEN_SIZEOF,
    TOKEN_TYPEDEF,
    TOKEN_EXTERN,
    TOKEN_STATIC,
    TOKEN_AUTO,
    TOKEN_REGISTER,
    TOKEN_INLINE,
    TOKEN_BOOL, // _Bool
    TOKEN_CHAR,
    TOKEN_SHORT,
    TOKEN_INT,
    TOKEN_LONG,
    TOKEN_SIGNED,
    TOKEN_UNSIGNED,
    TOKEN_FLOAT,
    TOKEN_DOUBLE,
    TOKEN_VOID,
    TOKEN_COMPLEX,   // _Complex
    TOKEN_IMAGINARY, // _Imaginary
    TOKEN_CONST,
    TOKEN_VOLATILE,
    TOKEN_RESTRICT,
    TOKEN_ATOMIC,
    TOKEN_STRUCT,
    TOKEN_UNION,
    TOKEN_ENUM,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_SWITCH,
    TOKEN_WHILE,
    TOKEN_DO,
    TOKEN_FOR,
    TOKEN_GOTO,
    TOKEN_CONTINUE,
    TOKEN_BREAK,
    TOKEN_RETURN,
    TOKEN_ALIGNAS,       // _Alignas
    TOKEN_ALIGNOF,       // _Alignof
    TOKEN_GENERIC,       // _Generic
    TOKEN_NORETURN,      // _Noreturn
    TOKEN_STATIC_ASSERT, // _Static_assert
    TOKEN_THREAD_LOCAL,  // _Thread_local
    TOKEN_KEYWORDS_END,

    // Punctuation
    TOKEN_SEMICOLON = TOKEN_KEYWORDS_END,
    TOKEN_LBRACKET, // (
    TOKEN_RBRACKET, // )
    TOKEN_LBRACE,   // {
    TOKEN_RBRACE,   // }
    TOKEN_LINDEX,   // [
    TOKEN_RINDEX,   // ]
    TOKEN_DOT,
    TOKEN_AND,  // &
    TOKEN_OR,   // |
    TOKEN_XOR,  // ^
    TOKEN_NOT,  // !
    TOKEN_BNOT, // ~
    TOKEN_SUB,
    TOKEN_ADD,
    TOKEN_ASTERISK,
    TOKEN_DIV,
    TOKEN_MOD,
    TOKEN_LT,     // <
    TOKEN_GT,     // >
    TOKEN_QMARK,  // ?
    TOKEN_COLON,  // :
    TOKEN_ASSIGN, // =
    TOKEN_COMMA,
    TOKEN_PTR_OP, // ->
    TOKEN_INC,
    TOKEN_DEC,
    TOKEN_LSHIFT,  // <<
    TOKEN_RSHIFT, // >>
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_EQ,
    TOKEN_NE,
    TOKEN_LAND, // &&
    TOKEN_LOR,  // ||
    TOKEN_MUL_ASSIGN,
    TOKEN_DIV_ASSIGN,
    TOKEN_MOD_ASSIGN,
    TOKEN_ADD_ASSIGN,
    TOKEN_SUB_ASSIGN,
    TOKEN_LSHIFT_ASSIGN,
    TOKEN_RSHIFT_ASSIGN,
    TOKEN_AND_ASSIGN,
    TOKEN_OR_ASSIGN,
    TOKEN_XOR_ASSIGN,
    TOKEN_ELLIPSIS, // ...

    // Preprocessor only
    TOKEN_PP_STRINGIFY, // #
    TOKEN_PP_CONCAT,    // ##

    TOKEN_INVALID
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

