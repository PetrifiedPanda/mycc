#ifndef PARSER_ERR_H
#define PARSER_ERR_H

#include <stdio.h>

#include "frontend/err_base.h"
#include "frontend/expected_tokens_err.h"
#include "frontend/token.h"

enum parser_err_kind {
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
};

struct parser_err {
    enum parser_err_kind kind;
    struct err_base base;
    union {
        struct expected_tokens_err expected_tokens_err; 
        struct { // redefined symbol
            struct str redefined_symbol;
            bool was_typedef_name;
            size_t prev_def_file;
            struct file_loc prev_def_loc;
        };
        struct { // incompatible type specs
            enum token_kind type_spec, prev_type_spec;
        };
        // disallowed type specs
        enum token_kind incompatible_type;
        struct str non_typedef_spelling;
    };
};

struct parser_err create_parser_err(void);

void set_parser_err(struct parser_err* err,
                    enum parser_err_kind kind,
                    struct source_loc loc);

void print_parser_err(FILE* out,
                      const struct file_info* file_info,
                      const struct parser_err* err);

void free_parser_err(struct parser_err* err);

#endif

