#ifndef PARSER_ERR_H
#define PARSER_ERR_H

#include <stdio.h>

#include "frontend/ErrBase.h"
#include "frontend/ExpectedTokensErr.h"
#include "frontend/Token.h"

typedef enum {
    PARSER_ERR_NONE = 0,
    PARSER_ERR_EXPECTED_TOKENS,
    PARSER_ERR_REDEFINED_SYMBOL,
    PARSER_ERR_ARR_DOUBLE_STATIC,
    PARSER_ERR_ARR_STATIC_NO_LEN,
    PARSER_ERR_ARR_STATIC_ASTERISK,
    PARSER_ERR_TYPEDEF_INIT,
    PARSER_ERR_TYPEDEF_FUNC_DEF,
    PARSER_ERR_TYPEDEF_PARAM_DECL,
    PARSER_ERR_TYPEDEF_STRUCT,
    PARSER_ERR_EMPTY_STRUCT_DECLARATOR,
    PARSER_ERR_INCOMPATIBLE_TYPE_SPECS,
    PARSER_ERR_TOO_MUCH_LONG,
    PARSER_ERR_DISALLOWED_TYPE_QUALS,
    PARSER_ERR_EXPECTED_TYPEDEF_NAME,
    PARSER_ERR_EMPTY_DIRECT_ABS_DECL,
} ParserErrKind;

typedef struct {
    ParserErrKind kind;
    ErrBase base;
    union {
        ExpectedTokensErr expected_tokens_err; 
        struct { // redefined symbol
            Str redefined_symbol;
            bool was_typedef_name;
            size_t prev_def_file;
            FileLoc prev_def_loc;
        };
        struct { // incompatible type specs
            TokenKind type_spec, prev_type_spec;
        };
        // disallowed type specs
        TokenKind incompatible_type;
        Str non_typedef_spelling;
    };
} ParserErr;

ParserErr create_parser_err(void);

void set_parser_err(ParserErr* err, ParserErrKind kind, SourceLoc loc);

void print_parser_err(FILE* out, const FileInfo* file_info, const ParserErr* err);

void free_parser_err(ParserErr* err);

#endif

