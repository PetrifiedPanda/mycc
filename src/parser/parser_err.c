#include "parser/parser_err.h"

#include "token_type.h"

#include "util/mem.h"
#include "util/annotations.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct parser_err create_parser_err(void) {
    return (struct parser_err){
        .type = PARSER_ERR_NONE,
        .base.loc.file = NULL,
    };
}

void set_parser_err(struct parser_err* err,
                    enum parser_err_type type,
                    struct source_loc* loc) {
    assert(err);
    assert(type != PARSER_ERR_NONE);
    assert(err->type == PARSER_ERR_NONE);

    err->type = type;
    err->base = create_err_base(loc);
}

void set_parser_err_copy(struct parser_err* err,
                         enum parser_err_type type,
                         const struct source_loc* loc) {
    assert(err);
    assert(type != PARSER_ERR_NONE);
    assert(err->type == PARSER_ERR_NONE);
    assert(loc);

    err->type = type;
    err->base = create_err_base_copy(loc);
}

void print_parser_err(FILE* out, const struct parser_err* err) {
    assert(err->type != PARSER_ERR_NONE);

    print_err_base(out, &err->base);
    switch (err->type) {
        case PARSER_ERR_NONE:
            UNREACHABLE();
        case PARSER_ERR_EXPECTED_TOKENS: {
            fprintf(out,
                    "Expected token of type %s",
                    get_type_str(err->expected[0]));
            for (size_t i = 1; i < err->num_expected; ++i) {
                printf(", %s", get_type_str(err->expected[i]));
            }

            if (err->got == INVALID) {
                fprintf(out, " but got to enf of file");
            } else {
                fprintf(out,
                        " but got token of type %s",
                        get_type_str(err->got));
            }
            break;
        }
        case PARSER_ERR_REDEFINED_SYMBOL: {
            const char* type_str = err->was_typedef_name ? "typedef name"
                                                         : "enum constant";
            fprintf(out,
                    "Redefined symbol %s that was already defined as %s in "
                    "%s(%zu, %zu)",
                    err->redefined_symbol,
                    type_str,
                    err->prev_def_file,
                    err->prev_def_loc.line,
                    err->prev_def_loc.index);
            break;
        }
        case PARSER_ERR_ARR_DOUBLE_STATIC:
            fprintf(out, "Expected only one use of static");
            break;
        case PARSER_ERR_ARR_STATIC_NO_LEN:
            fprintf(out, "Expected array length after use of static");
            break;
        case PARSER_ERR_ARR_STATIC_ASTERISK:
            fprintf(out, "Asterisk cannot be used with static");
            break;
        case PARSER_ERR_TYPEDEF_INIT:
            fprintf(out, "Initializer not allowed in typedef");
            break;
        case PARSER_ERR_TYPEDEF_FUNC_DEF:
            fprintf(out, "Function definition declared typedef");
            break;
        case PARSER_ERR_TYPEDEF_PARAM_DECL:
            fprintf(out, "typedef is not allowed in function declarator");
            break;
        case PARSER_ERR_TYPEDEF_STRUCT:
            fprintf(out, "typedef is not allowed in struct declaration");
            break;
        case PARSER_ERR_EMPTY_STRUCT_DECLARATOR:
            fprintf(out, "Expected a declarator or a bit field specifier");
            break;
        case PARSER_ERR_INCOMPATIBLE_TYPE_SPECS:
            fprintf(out,
                    "Cannot combine %s with previous %s type specifier",
                    get_type_str(err->type_spec),
                    get_type_str(err->prev_type_spec));
            break;
        case PARSER_ERR_DISALLOWED_TYPE_QUALS:
            fprintf(out,
                    "Cannot add qualifiers to type %s",
                    get_type_str(err->incompatible_type));
            break;
        case PARSER_ERR_EXPECTED_TYPEDEF_NAME:
            fprintf(
                out,
                "Expected a typedef name but got identifier with spelling %s",
                err->non_typedef_spelling);
    }
    fprintf(out, "\n");
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
        case PARSER_ERR_EXPECTED_TYPEDEF_NAME:
            free(err->non_typedef_spelling);
            break;
        default:
            break;
    }
}

