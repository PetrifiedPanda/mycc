#include "frontend/parser/ParserErr.h"

#include <stdio.h>
#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

ParserErr ParserErr_create(void) {
    return (ParserErr){
        .kind = PARSER_ERR_NONE,
    };
}

void ParserErr_set(ParserErr* err, ParserErrKind kind, SourceLoc loc) {
    assert(err);
    assert(kind != PARSER_ERR_NONE);
    assert(err->kind == PARSER_ERR_NONE);

    err->kind = kind;
    err->base = ErrBase_create(loc);
}

void ParserErr_print(FILE* out, const FileInfo* file_info, const ParserErr* err) {
    assert(err->kind != PARSER_ERR_NONE);

    ErrBase_print(out, file_info, &err->base);
    switch (err->kind) {
        case PARSER_ERR_NONE:
            UNREACHABLE();
        case PARSER_ERR_EXPECTED_TOKENS: {
            ExpectedTokensErr_print(out, &err->expected_tokens_err);
            break;
        }
        case PARSER_ERR_REDEFINED_SYMBOL: {
            assert(err->prev_def_file < file_info->len);
            const char* path = Str_get_data(&file_info->paths[err->prev_def_file]);
            const char* type_str = err->was_typedef_name ? "typedef name"
                                                         : "enum constant";
            fprintf(out,
                    "Redefined symbol %s that was already defined as %s in "
                    "%s(%zu, %zu)",
                    Str_get_data(&err->redefined_symbol),
                    type_str,
                    path,
                    err->prev_def_loc.line,
                    err->prev_def_loc.index);
            break;
        }
        case PARSER_ERR_ARR_DOUBLE_STATIC:
            fputs("Expected only one use of static", out);
            break;
        case PARSER_ERR_ARR_STATIC_NO_LEN:
            fputs("Expected array length after use of static", out);
            break;
        case PARSER_ERR_ARR_STATIC_ASTERISK:
            fputs("Asterisk cannot be used with static", out);
            break;
        case PARSER_ERR_TYPEDEF_INIT:
            fputs("Initializer not allowed in typedef", out);
            break;
        case PARSER_ERR_TYPEDEF_FUNC_DEF:
            fputs("Function definition declared typedef", out);
            break;
        case PARSER_ERR_TYPEDEF_PARAM_DECL:
            fputs("typedef is not allowed in function declarator", out);
            break;
        case PARSER_ERR_TYPEDEF_STRUCT:
            fputs("typedef is not allowed in struct declaration", out);
            break;
        case PARSER_ERR_EMPTY_STRUCT_DECLARATOR:
            fputs("Expected a declarator or a bit field specifier", out);
            break;
        case PARSER_ERR_INCOMPATIBLE_TYPE_SPECS:
            fprintf(out,
                    "Cannot combine %s with previous %s type specifier",
                    TokenKind_str(err->type_spec),
                    TokenKind_str(err->prev_type_spec));
            break;
        case PARSER_ERR_TOO_MUCH_LONG:
            fputs("More than 2 long specifiers are not allowed", out);
            break;
        case PARSER_ERR_DISALLOWED_TYPE_QUALS:
            fprintf(out,
                    "Cannot add qualifiers to type %s",
                    TokenKind_str(err->incompatible_type));
            break;
        case PARSER_ERR_EXPECTED_TYPEDEF_NAME:
            fprintf(
                out,
                "Expected a typedef name but got identifier with spelling %s",
                Str_get_data(&err->non_typedef_spelling));
            break;
        case PARSER_ERR_EMPTY_DIRECT_ABS_DECL:
            fputs("Empty abstract declarator", out);
            break;
    }
    fputc('\n', out);
}

void ParserErr_free(ParserErr* err) {
    switch (err->kind) {
        case PARSER_ERR_REDEFINED_SYMBOL:
            Str_free(&err->redefined_symbol);
            break;
        case PARSER_ERR_EXPECTED_TYPEDEF_NAME:
            Str_free(&err->non_typedef_spelling);
            break;
        default:
            break;
    }
}
