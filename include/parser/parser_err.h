#ifndef PARSER_ERR_H
#define PARSER_ERR_H

#include <stdio.h>

#include "err_base.h"
#include "token.h"

enum parser_err_type {
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
    PARSER_ERR_DISALLOWED_TYPE_QUALS,
    PARSER_ERR_EXPECTED_TYPEDEF_NAME,
};

struct parser_err {
    enum parser_err_type type;
    struct err_base base;
    union {
        struct { // expected tokens
            enum token_type got;
            size_t num_expected;
            enum token_type* expected;
        };
        struct { // redefined symbol
            char* redefined_symbol;
            bool was_typedef_name;
            char* prev_def_file;
            struct file_loc prev_def_loc;
        };
        struct { // incompatible type specs
            enum token_type type_spec, prev_type_spec;
        };
        // disallowed type specs
        enum token_type incompatible_type;
        char* non_typedef_spelling;
    };
};

struct parser_err create_parser_err(void);

void set_parser_err(struct parser_err* err,
                    enum parser_err_type type,
                    struct source_loc* loc);

void set_parser_err_copy(struct parser_err* err,
                         enum parser_err_type type,
                         const struct source_loc* loc);

void print_parser_err(FILE* out, const struct parser_err* err);

void free_parser_err(struct parser_err* err);

#endif

