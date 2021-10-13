#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>
#include <stdlib.h>

typedef enum  {
    IDENTIFIER,
    CONSTANT,
    STRING_LITERAL,
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
    CHAR,
    SHORT,
    INT,
    LONG,
    SIGNED,
    UNSIGNED,
    FLOAT,
    DOUBLE,
    CONST,
    VOLATILE,
    VOID,
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
} TokenType;


typedef struct {
    TokenType type;
    char* spelling;
    size_t source_line, source_char;
} Token;

bool create_token(Token* t, TokenType type, const char* spelling, size_t line, size_t idx);
void create_token_move(Token* t, TokenType type, char* spelling, size_t line, size_t idx);

void free_token(Token* t);

const char* get_spelling(TokenType type);
const char* get_type_str(TokenType type);

bool is_unary_op(TokenType t);
bool is_assign_op(TokenType t);
bool is_storage_class_spec(TokenType t);
bool is_keyword_type_spec(TokenType t);


#endif
