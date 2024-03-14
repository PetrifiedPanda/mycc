#include "frontend/preproc/PreprocErr.h"

#include <string.h>
#include <errno.h>
#include <assert.h>

#include "frontend/ErrBase.h"

#include "util/macro_util.h"

PreprocErr PreprocErr_create(void) {
    return (PreprocErr){
        .kind = PREPROC_ERR_NONE,
    };
}

void PreprocErr_set(PreprocErr* err, PreprocErrKind kind, SourceLoc loc) {
    assert(err);
    assert(kind != PREPROC_ERR_NONE);
    assert(err->kind == PREPROC_ERR_NONE);

    err->kind = kind;
    err->base = ErrBase_create(loc);
}

static Str get_single_macro_op_str(SingleMacroOpKind type);
static Str get_else_op_str(ElseOpKind type);

void PreprocErr_print(File out, const FileInfo* file_info, PreprocErr* err) {
    assert(err->kind != PREPROC_ERR_NONE);

    switch (err->kind) {
        case PREPROC_ERR_NONE:
            UNREACHABLE();
            break;
        case PREPROC_ERR_OPEN_FILE:
            if (err->base.loc.file_idx != UINT32_MAX) {
                ErrBase_print(out, file_info, &err->base);
            }

            const Str fail_path = StrBuf_as_str(&err->fail_filename);
            const char* err_string = strerror(err->errno_state);
            File_printf(out,
                        "Failed to open file {Str}: {c_str}",
                        fail_path,
                        err_string);
            break;
        case PREPROC_ERR_UNTERMINATED_LIT:
            ErrBase_print(out, file_info, &err->base);
            File_printf(out,
                        "{Str} literal not properly terminated",
                        err->is_char_lit ? STR_LIT("Char") : STR_LIT("String"));
            break;
        case PREPROC_ERR_INVALID_ID:
            ErrBase_print(out, file_info, &err->base);
            File_printf(out,
                        "Invalid identifier: {Str}",
                        StrBuf_as_str(&err->invalid_id));
            break;
        case PREPROC_ERR_INVALID_NUMBER:
            ErrBase_print(out, file_info, &err->base);
            File_printf(out,
                        "Invalid number: {Str}",
                        StrBuf_as_str(&err->invalid_num));
            break;
        case PREPROC_ERR_MACRO_ARG_COUNT:
            ErrBase_print(out, file_info, &err->base);
            if (err->too_few_args) {
                File_printf(
                    out,
                    "Too few arguments in function-like macro invocation: "
                    "Expected{Str} {u32} arguments",
                    err->is_variadic ? STR_LIT(" at least") : STR_LIT(""),
                    err->expected_arg_count);
            } else {
                File_printf(
                    out,
                    "Too many arguments in function like macro invocation: "
                    "Expected only {u32} arguments",
                    err->expected_arg_count);
            }
            break;
        case PREPROC_ERR_UNTERMINATED_MACRO:
            ErrBase_print(out, file_info, &err->base);
            File_put_str("Unterminated macro", out);
            break;
        case PREPROC_ERR_UNTERMINATED_COND: {
            ErrBase_print(out, file_info, &err->base);
            const SourceLoc* loc = &err->unterminated_cond_loc;
            const Str cond_file = FileInfo_get(file_info, loc->file_idx);
            File_printf(
                out,
                "Conditional started at {Str}:{u32},{u32} not terminated",
                cond_file,
                loc->file_loc.line,
                loc->file_loc.index);
            break;
        }
        case PREPROC_ERR_ARG_COUNT: {
            ErrBase_print(out, file_info, &err->base);
            Str dir_str = get_single_macro_op_str(err->count_dir_kind);
            if (err->count_empty) {
                File_printf(out,
                            "Expected an identifier after {Str} directive",
                            dir_str);
            } else {
                File_printf(out,
                            "Excess tokens after {Str} directive",
                            dir_str);
            }
            break;
        }
        case PREPROC_ERR_IFDEF_NOT_ID: {
            ErrBase_print(out, file_info, &err->base);
            Str dir_str = get_single_macro_op_str(err->not_identifier_op);
            File_printf(
                out,
                "Expected an identifier after {Str} directive, but got {Str}",
                dir_str,
                TokenKind_str(err->not_identifier_got));
            break;
        }
        case PREPROC_ERR_MISSING_IF: {
            ErrBase_print(out, file_info, &err->base);
            Str dir_str = get_else_op_str(err->missing_if_op);
            File_printf(out, "{Str} directive without if", dir_str);
            break;
        }
        case PREPROC_ERR_INVALID_PREPROC_DIR:
            ErrBase_print(out, file_info, &err->base);
            File_put_str("Invalid preprocessor directive", out);
            break;
        case PREPROC_ERR_ELIF_ELSE_AFTER_ELSE: {
            assert(err->elif_after_else_op != ELSE_OP_ENDIF);
            ErrBase_print(out, file_info, &err->base);
            Str prev_else_file = FileInfo_get(file_info,
                                              err->prev_else_loc.file_idx);
            const FileLoc loc = err->prev_else_loc.file_loc;
            switch (err->elif_after_else_op) {
                case ELSE_OP_ELIF:
                    File_printf(out,
                                "elif directive after else directive in "
                                "{Str}:({u32},{u32})",
                                prev_else_file,
                                loc.line,
                                loc.index);
                    break;
                case ELSE_OP_ELSE:
                    File_printf(out,
                                "Second else directive after else in "
                                "{Str}:({u32},{u32})",
                                prev_else_file,
                                loc.line,
                                loc.index);
                    break;
                default:
                    UNREACHABLE();
            }
            break;
        }
        case PREPROC_ERR_MISPLACED_PREPROC_TOKEN:
            assert(err->misplaced_preproc_tok == TOKEN_PP_STRINGIFY
                   || err->misplaced_preproc_tok == TOKEN_PP_CONCAT);
            File_printf(out,
                        "preprocessor token \"{Str}\" outside of preprocessor "
                        "directive",
                        TokenKind_get_spelling(err->misplaced_preproc_tok));
            break;
        case PREPROC_ERR_INT_CONST:
            assert(StrBuf_valid(&err->constant_spell));
            ErrBase_print(out, file_info, &err->base);
            File_printf(
                out,
                "Integer constant {Str} is not a valid integer constant",
                StrBuf_as_str(&err->constant_spell));
            IntConstErr_print(out, &err->int_const_err);
            break;
        case PREPROC_ERR_FLOAT_CONST:
            assert(StrBuf_valid(&err->constant_spell));
            ErrBase_print(out, file_info, &err->base);
            File_printf(
                out,
                "Floating constant {Str} is not a valid integer constant",
                StrBuf_as_str(&err->constant_spell));
            FloatConstErr_print(out, &err->float_const_err);
            break;
        case PREPROC_ERR_CHAR_CONST:
            assert(StrBuf_valid(&err->constant_spell));
            ErrBase_print(out, file_info, &err->base);
            File_printf(
                out,
                "Character constant {Str} is not a valid character constant",
                StrBuf_as_str(&err->constant_spell));
            CharConstErr_print(out, &err->char_const_err);
            break;
        case PREPROC_ERR_EMPTY_DEFINE:
            ErrBase_print(out, file_info, &err->base);
            File_put_str("Empty define directive", out);
            break;
        case PREPROC_ERR_DEFINE_NOT_ID:
            ErrBase_print(out, file_info, &err->base);
            File_put_str("define not followed by id", out);
            break;
        case PREPROC_ERR_EXPECTED_TOKENS:
            ErrBase_print(out, file_info, &err->base);
            ExpectedTokensErr_print(out, &err->expected_tokens_err);
            break;
        case PREPROC_ERR_DUPLICATE_MACRO_PARAM:
            ErrBase_print(out, file_info, &err->base);
            File_printf(out,
                        "Duplicate macro argument name \"{Str}\"",
                        StrBuf_as_str(&err->duplicate_arg_name));
            break;
        case PREPROC_ERR_INVALID_BACKSLASH:
            ErrBase_print(out, file_info, &err->base);
            File_put_str("Backslash \'\\\' only allowed at the end of a line",
                         out);
            break;
        case PREPROC_ERR_INCLUDE_NUM_ARGS:
            ErrBase_print(out, file_info, &err->base);
            File_put_str("Include directive must have exactly one argument",
                         out);
            break;
        case PREPROC_ERR_INCLUDE_NOT_STRING_LITERAL:
            ErrBase_print(out, file_info, &err->base);
            File_put_str(
                "Include directive only takes '\"' or '<' '>' literals",
                out);
            break;
        case PREPROC_ERR_INCOMPLETE_EXPR:
            ErrBase_print(out, file_info, &err->base);
            File_put_str("Incomplete preprocessor constant expression", out);
            break;
    }
    File_putc('\n', out);
}

static Str get_single_macro_op_str(SingleMacroOpKind type) {
    switch (type) {
        case SINGLE_MACRO_OP_IFDEF:
            return STR_LIT("ifdef");
        case SINGLE_MACRO_OP_IFNDEF:
            return STR_LIT("ifndef");
        case SINGLE_MACRO_OP_UNDEF:
            return STR_LIT("undef");
    }

    UNREACHABLE();
}

static Str get_else_op_str(ElseOpKind type) {
    switch (type) {
        case ELSE_OP_ELIF:
            return STR_LIT("elif");
        case ELSE_OP_ELSE:
            return STR_LIT("else");
        case ELSE_OP_ENDIF:
            return STR_LIT("endif");
    }

    UNREACHABLE();
}

void PreprocErr_set_file_err(PreprocErr* err,
                             const StrBuf* fail_filename,
                             SourceLoc include_loc) {
    assert(fail_filename);
    assert(StrBuf_valid(fail_filename));

    PreprocErr_set(err, PREPROC_ERR_OPEN_FILE, include_loc);
    err->errno_state = errno;
    err->fail_filename = *fail_filename;
    errno = 0;
}

void PreprocErr_free(PreprocErr* err) {
    assert(err);

    switch (err->kind) {
        case PREPROC_ERR_INVALID_ID:
            StrBuf_free(&err->invalid_id);
            break;
        case PREPROC_ERR_INVALID_NUMBER:
            StrBuf_free(&err->invalid_num);
            break;
        case PREPROC_ERR_INT_CONST:
        case PREPROC_ERR_FLOAT_CONST:
        case PREPROC_ERR_CHAR_CONST:
            StrBuf_free(&err->constant_spell);
            break;
        case PREPROC_ERR_DUPLICATE_MACRO_PARAM:
            StrBuf_free(&err->duplicate_arg_name);
            break;
        case PREPROC_ERR_OPEN_FILE:
            StrBuf_free(&err->fail_filename);
            break;
        case PREPROC_ERR_MACRO_ARG_COUNT:
        case PREPROC_ERR_NONE:
        case PREPROC_ERR_UNTERMINATED_LIT:
        case PREPROC_ERR_UNTERMINATED_MACRO:
        case PREPROC_ERR_UNTERMINATED_COND:
        case PREPROC_ERR_ARG_COUNT:
        case PREPROC_ERR_IFDEF_NOT_ID:
        case PREPROC_ERR_MISSING_IF:
        case PREPROC_ERR_INVALID_PREPROC_DIR:
        case PREPROC_ERR_ELIF_ELSE_AFTER_ELSE:
        case PREPROC_ERR_MISPLACED_PREPROC_TOKEN:
        case PREPROC_ERR_EMPTY_DEFINE:
        case PREPROC_ERR_DEFINE_NOT_ID:
        case PREPROC_ERR_EXPECTED_TOKENS:
        case PREPROC_ERR_INVALID_BACKSLASH:
        case PREPROC_ERR_INCLUDE_NUM_ARGS:
        case PREPROC_ERR_INCLUDE_NOT_STRING_LITERAL:
        case PREPROC_ERR_INCOMPLETE_EXPR:
            break;
    }
}
