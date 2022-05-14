#include "parser/parser_err.h"
#include "token_type.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct parser_err create_parser_err() {
    return (struct parser_err){
        .type = PARSER_ERR_NONE,
    };
}

void init_parser_err(struct parser_err* err,
                     enum parser_err_type type,
                     char* file,
                     struct source_location loc) {
    assert(err);
    assert(type != PARSER_ERR_NONE);
    assert(err->type == PARSER_ERR_NONE);

    err->type = type;
    err->base = (struct err_base){
        .file = file,
        .loc = loc,
    };
}



void print_parser_err(const struct parser_err* err) {
    assert(err->type != PARSER_ERR_NONE);

    print_err_base(&err->base);
    switch (err->type) {
        case PARSER_ERR_EXPECTED_TOKENS: {
            printf("Expected token of type %s", get_type_str(err->expected[0]));
            for (size_t i = 1; i < err->num_expected; ++i) {
                printf(", %s", get_type_str(err->expected[i]));
            }
            
            if (err->got == INVALID) {
                printf(" but got to enf of file");
            } else {
                printf(" but got token of type %s", get_type_str(err->got));
            }
            break;
        }
        case PARSER_ERR_REDEFINED_SYMBOL: {
            const char* type_str = err->was_typedef_name ? "typedef name" : "enum constant";
            printf("Redefined symbol %s that was already defined as %s in %s(%zu, %zu)",
                   err->redefined_symbol,
                   type_str,
                   err->prev_def_file,
                   err->prev_def_loc.line,
                   err->prev_def_loc.index);
            break;
        }
        default:
            assert(false);
    }
}

void free_parser_err(struct parser_err* err) {
    free_err_base(&err->base);

    switch (err->type) {
        case PARSER_ERR_EXPECTED_TOKENS:
            free(err->expected);
            break;
        case PARSER_ERR_REDEFINED_SYMBOL:
            free(err->redefined_symbol);
            free(err->prev_def_file);
            break;
        default:
            break;
    }
}

