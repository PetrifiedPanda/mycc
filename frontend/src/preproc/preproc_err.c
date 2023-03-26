#include "frontend/preproc/preproc_err.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "frontend/err_base.h"

#include "util/mem.h"
#include "util/macro_util.h"

struct preproc_err create_preproc_err(void) {
    return (struct preproc_err){
        .kind = PREPROC_ERR_NONE,
    };
}

void set_preproc_err(struct preproc_err* err,
                     enum preproc_err_kind kind,
                     struct source_loc loc) {
    assert(err);
    assert(kind != PREPROC_ERR_NONE);
    assert(err->kind == PREPROC_ERR_NONE);

    err->kind = kind;
    err->base = create_err_base(loc);
}

static const char* get_single_macro_op_str(enum single_macro_op_kind type);
static const char* get_else_op_str(enum else_op_kind type);

void print_preproc_err(FILE* out,
                       const struct file_info* file_info,
                       struct preproc_err* err) {
    assert(err->kind != PREPROC_ERR_NONE);

    switch (err->kind) {
        case PREPROC_ERR_NONE:
            UNREACHABLE();
            break;
        case PREPROC_ERR_OPEN_FILE:
            if (err->base.loc.file_idx != (size_t)-1) {
                print_err_base(out, file_info, &err->base);
            }

            const char* fail_path = str_get_data(&err->fail_filename);
            const char* err_string = strerror(err->errno_state);
            fprintf(out, "Failed to open file %s: %s", fail_path, err_string);
            break;
        case PREPROC_ERR_UNTERMINATED_LIT:
            print_err_base(out, file_info, &err->base);
            fprintf(out,
                    "%s literal not properly terminated",
                    err->is_char_lit ? "Char" : "String");
            break;
        case PREPROC_ERR_INVALID_ID:
            print_err_base(out, file_info, &err->base);
            fprintf(out,
                    "Invalid identifier: %s",
                    str_get_data(&err->invalid_id));
            break;
        case PREPROC_ERR_INVALID_NUMBER:
            print_err_base(out, file_info, &err->base);
            fprintf(out, "Invalid number: %s", str_get_data(&err->invalid_num));
            break;
        case PREPROC_ERR_MACRO_ARG_COUNT:
            print_err_base(out, file_info, &err->base);
            if (err->too_few_args) {
                fprintf(out,
                        "Too few arguments in function-like macro invocation: "
                        "Expected%s %zu arguments",
                        err->is_variadic ? " at least" : "",
                        err->expected_arg_count);
            } else {
                fprintf(out,
                        "Too many arguments in function like macro invocation: "
                        "Expected only %zu arguments",
                        err->expected_arg_count);
            }
            break;
        case PREPROC_ERR_UNTERMINATED_MACRO:
            print_err_base(out, file_info, &err->base);
            fputs("Unterminated macro", out);
            break;
        case PREPROC_ERR_UNTERMINATED_COND: {
            print_err_base(out, file_info, &err->base);
            const struct source_loc* loc = &err->unterminated_cond_loc;
            const char* cond_file = str_get_data(
                &file_info->paths[loc->file_idx]);
            fprintf(out,
                    "Conditional started at %s:%zu,%zu not terminated",
                    cond_file,
                    loc->file_loc.line,
                    loc->file_loc.index);
            break;
        }
        case PREPROC_ERR_ARG_COUNT: {
            print_err_base(out, file_info, &err->base);
            const char* dir_str = get_single_macro_op_str(err->count_dir_kind);
            if (err->count_empty) {
                fprintf(out,
                        "Expected an identifier after %s directive",
                        dir_str);
            } else {
                fprintf(out, "Excess tokens after %s directive", dir_str);
            }
            break;
        }
        case PREPROC_ERR_IFDEF_NOT_ID: {
            print_err_base(out, file_info, &err->base);
            const char* dir_str = get_single_macro_op_str(
                err->not_identifier_op);
            fprintf(out,
                    "Expected an identifier after %s directive, but got %s",
                    dir_str,
                    get_token_kind_str(err->not_identifier_got));
            break;
        }
        case PREPROC_ERR_MISSING_IF: {
            print_err_base(out, file_info, &err->base);
            const char* dir_str = get_else_op_str(err->missing_if_op);
            fprintf(out, "%s directive without if", dir_str);
            break;
        }
        case PREPROC_ERR_INVALID_PREPROC_DIR:
            print_err_base(out, file_info, &err->base);
            fputs("Invalid preprocessor directive", out);
            break;
        case PREPROC_ERR_ELIF_ELSE_AFTER_ELSE: {
            assert(err->elif_after_else_op != ELSE_OP_ENDIF);
            print_err_base(out, file_info, &err->base);
            const char* prev_else_file = str_get_data(
                &file_info->paths[err->prev_else_loc.file_idx]);
            const struct file_loc loc = err->prev_else_loc.file_loc;
            switch (err->elif_after_else_op) {
                case ELSE_OP_ELIF:
                    fprintf(
                        out,
                        "elif directive after else directive in %s:(%zu,%zu)",
                        prev_else_file,
                        loc.line,
                        loc.index);
                    break;
                case ELSE_OP_ELSE:
                    fprintf(out,
                            "Second else directive after else in %s:(%zu,%zu)",
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
            fprintf(
                out,
                "preprocessor token \"%s\" outside of preprocessor directive",
                get_token_kind_spelling(err->misplaced_preproc_tok));
            break;
        case PREPROC_ERR_INT_CONST:
            assert(str_is_valid(&err->constant_spell));
            print_err_base(out, file_info, &err->base);
            fprintf(out,
                    "Integer constant %s is not a valid integer constant",
                    str_get_data(&err->constant_spell));
            print_int_const_err(out, &err->int_const_err);
            break;
        case PREPROC_ERR_FLOAT_CONST:
            assert(str_is_valid(&err->constant_spell));
            print_err_base(out, file_info, &err->base);
            fprintf(out,
                    "Floating constant %s is not a valid integer constant",
                    str_get_data(&err->constant_spell));
            print_float_const_err(out, &err->float_const_err);
            break;
        case PREPROC_ERR_CHAR_CONST:
            assert(str_is_valid(&err->constant_spell));
            print_err_base(out, file_info, &err->base);
            fprintf(out,
                    "Character constant %s is not a valid character constant",
                    str_get_data(&err->constant_spell));
            print_char_const_err(out, &err->char_const_err);
            break;
        case PREPROC_ERR_EMPTY_DEFINE:
            print_err_base(out, file_info, &err->base);
            fputs("Empty define directive", out);
            break;
        case PREPROC_ERR_DEFINE_NOT_ID:
            print_err_base(out, file_info, &err->base);
            fputs("define not followed by id", out);
            break;
        case PREPROC_ERR_EXPECTED_TOKENS:
            print_err_base(out, file_info, &err->base);
            print_expected_tokens_err(out, &err->expected_tokens_err);
            break;
        case PREPROC_ERR_DUPLICATE_MACRO_PARAM:
            print_err_base(out, file_info, &err->base);
            fprintf(out,
                    "Duplicate macro argument name \"%s\"",
                    str_get_data(&err->duplicate_arg_name));
            break;
        case PREPROC_ERR_INVALID_BACKSLASH:
            print_err_base(out, file_info, &err->base);
            fputs("Backslash \'\\\' only allowed at the end of a line", out);
            break;
    }
    fputc('\n', out);
}

static const char* get_single_macro_op_str(enum single_macro_op_kind type) {
    switch (type) {
        case SINGLE_MACRO_OP_IFDEF:
            return "ifdef";
        case SINGLE_MACRO_OP_IFNDEF:
            return "ifndef";
        case SINGLE_MACRO_OP_UNDEF:
            return "undef";
    }

    UNREACHABLE();
}

static const char* get_else_op_str(enum else_op_kind type) {
    switch (type) {
        case ELSE_OP_ELIF:
            return "elif";
        case ELSE_OP_ELSE:
            return "else";
        case ELSE_OP_ENDIF:
            return "endif";
    }

    UNREACHABLE();
}

void set_preproc_file_err(struct preproc_err* err,
                          const struct str* fail_filename,
                          struct source_loc include_loc) {
    assert(fail_filename);
    assert(str_is_valid(fail_filename));

    set_preproc_err(err, PREPROC_ERR_OPEN_FILE, include_loc);
    err->errno_state = errno;
    err->fail_filename = *fail_filename;
    errno = 0;
}

void free_preproc_err(struct preproc_err* err) {
    assert(err);

    switch (err->kind) {
        case PREPROC_ERR_INVALID_ID:
            free_str(&err->invalid_id);
            break;
        case PREPROC_ERR_INVALID_NUMBER:
            free_str(&err->invalid_num);
            break;
        case PREPROC_ERR_INT_CONST:
        case PREPROC_ERR_FLOAT_CONST:
        case PREPROC_ERR_CHAR_CONST:
            free_str(&err->constant_spell);
            break;
        case PREPROC_ERR_DUPLICATE_MACRO_PARAM:
            free_str(&err->duplicate_arg_name);
            break;
        case PREPROC_ERR_OPEN_FILE:
            free_str(&err->fail_filename);
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
            break;
    }
}

