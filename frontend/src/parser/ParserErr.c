#include "frontend/parser/ParserErr.h"

#include <assert.h>

#include "util/macro_util.h"

ParserErr ParserErr_create(void) {
    return (ParserErr){
        .kind = PARSER_ERR_NONE,
    };
}

void ParserErr_set(ParserErr* err, ParserErrKind kind, uint32_t idx) {
    assert(err);
    assert(kind != PARSER_ERR_NONE);
    assert(err->kind == PARSER_ERR_NONE);

    err->kind = kind;
    err->err_token_idx = idx;
}

void ParserErr_print(File out,
                     const FileInfo* file_info,
                     const TokenArr* tokens,
                     const ParserErr* err) {
    assert(err->kind != PARSER_ERR_NONE);

    ErrBase base = ErrBase_create(tokens->locs[err->err_token_idx]);
    ErrBase_print(out, file_info, &base);
    switch (err->kind) {
        case PARSER_ERR_NONE:
            UNREACHABLE();
        case PARSER_ERR_EXPECTED_TOKENS: {
            ExpectedTokensErr_print(out, &err->expected_tokens_err);
            break;
        }
        case PARSER_ERR_REDEFINED_SYMBOL: {
            assert(err->prev_def_idx < tokens->len);
            const SourceLoc loc = tokens->locs[err->prev_def_idx];
            const Str path = FileInfo_get(file_info, loc.file_idx);
            Str type_str = err->was_typedef_name ? STR_LIT("typedef name")
                                                 : STR_LIT("enum constant");
            File_printf(
                out,
                "Redefined symbol {Str} that was already defined as {Str} in "
                "{Str}({u32}, {u32})",
                StrBuf_as_str(&tokens->vals[err->err_token_idx].spelling),
                type_str,
                path,
                loc.file_loc.line,
                loc.file_loc.index);
            break;
        }
        case PARSER_ERR_ARR_DOUBLE_STATIC:
            File_put_str("Expected only one use of static", out);
            break;
        case PARSER_ERR_ARR_STATIC_NO_LEN:
            File_put_str("Expected array length after use of static", out);
            break;
        case PARSER_ERR_ARR_STATIC_ASTERISK:
            File_put_str("Asterisk cannot be used with static", out);
            break;
        case PARSER_ERR_TYPEDEF_INIT:
            File_put_str("Initializer not allowed in typedef", out);
            break;
        case PARSER_ERR_TYPEDEF_FUNC_DEF:
            File_put_str("Function definition declared typedef", out);
            break;
        case PARSER_ERR_TYPEDEF_PARAM_DECL:
            File_put_str("typedef is not allowed in function declarator", out);
            break;
        case PARSER_ERR_TYPEDEF_STRUCT:
            File_put_str("typedef is not allowed in struct declaration", out);
            break;
        case PARSER_ERR_EMPTY_STRUCT_DECLARATOR:
            File_put_str("Expected a declarator or a bit field specifier", out);
            break;
        case PARSER_ERR_INCOMPATIBLE_TYPE_SPECS:
            File_printf(
                out,
                "Cannot combine {Str} with previous {Str} type specifier",
                TokenKind_str(err->type_spec),
                TokenKind_str(err->prev_type_spec));
            break;
        case PARSER_ERR_TOO_MUCH_LONG:
            File_put_str("More than 2 long specifiers are not allowed", out);
            break;
        case PARSER_ERR_DISALLOWED_TYPE_QUALS:
            File_printf(out,
                        "Cannot add qualifiers to type {Str}",
                        TokenKind_str(err->incompatible_type));
            break;
        case PARSER_ERR_EXPECTED_TYPEDEF_NAME:
            File_printf(
                out,
                "Expected a typedef name but got identifier with "
                "spelling {Str}",
                StrBuf_as_str(&tokens->vals[err->err_token_idx].spelling));
            break;
        case PARSER_ERR_EMPTY_DIRECT_ABS_DECL:
            File_put_str("Empty abstract declarator", out);
            break;
        case PARSER_ERR_TYPEDEF_WITHOUT_DECLARATOR:
            File_put_str("Typedef without declarator", out);
            break;
    }
    File_putc('\n', out);
}
