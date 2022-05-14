#ifndef PARSER_ERR_H
#define PARSER_ERR_H

#include "err_base.h"
#include "token.h"

enum parser_err_type {
    PARSER_ERR_NONE = 0,
    PARSER_ERR_EXPECTED_TOKENS,
    PARSER_ERR_REDEFINED_SYMBOL,
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
            struct source_location prev_def_loc;
        };
    };
};

struct parser_err create_parser_err();

void init_parser_err(struct parser_err* err,
                     enum parser_err_type type,
                     char* file,
                     struct source_location loc);

void print_parser_err(const struct parser_err* err);

void free_parser_err(struct parser_err* err);

#endif

