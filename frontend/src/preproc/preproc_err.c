#include "frontend/preproc/preproc_err.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "frontend/err_base.h"

#include "util/mem.h"
#include "util/macro_util.h"

struct preproc_err create_preproc_err(void) {
    return (struct preproc_err){
        .type = PREPROC_ERR_NONE,
    };
}

void set_preproc_err(struct preproc_err* err,
                     enum preproc_err_type type,
                     struct source_loc loc) {
    assert(err);
    assert(type != PREPROC_ERR_NONE);
    assert(err->type == PREPROC_ERR_NONE);

    err->type = type;
    err->base = create_err_base(loc);
}

static const char* get_single_macro_op_str(enum single_macro_op_type type);
static const char* get_else_op_str(enum else_op_type type);

void print_preproc_err(FILE* out,
                       const struct file_info* file_info,
                       struct preproc_err* err) {
    assert(err->type != PREPROC_ERR_NONE);

    switch (err->type) {
        case PREPROC_ERR_NONE:
            UNREACHABLE();
            break;
        case PREPROC_ERR_FILE_FAIL:
            if (err->base.loc.file_idx != (size_t)-1) {
                print_err_base(out, file_info, &err->base);
            }

            assert(err->fail_file < file_info->len);
            const char* fail_path = str_get_data(
                &file_info->paths[err->fail_file]);
            const char* err_string = strerror(err->errno_state);
            if (err->open_fail) {
                fprintf(out,
                        "Failed to open file %s: %s",
                        fail_path,
                        err_string);
            } else {
                fprintf(out,
                        "Failed to close file %s: %s",
                        fail_path,
                        err_string);
            }
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
            fprintf(out, "Unterminated macro");
            break;
        case PREPROC_ERR_ARG_COUNT: {
            print_err_base(out, file_info, &err->base);
            const char* dir_str = get_single_macro_op_str(err->count_dir_type);
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
                    get_type_str(err->not_identifier_got));
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
            fprintf(out, "Invalid preprocessor directive");
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
            assert(err->misplaced_preproc_tok == STRINGIFY_OP
                   || err->misplaced_preproc_tok == CONCAT_OP);
            fprintf(
                out,
                "preprocessor token \"%s\" outside of preprocessor directive",
                get_spelling(err->misplaced_preproc_tok));
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
            fprintf(out, "Empty define directive");
            break;
        case PREPROC_ERR_DEFINE_NOT_ID:
            print_err_base(out, file_info, &err->base);
            fprintf(out, "define not followed by id");
            break;
        case PREPROC_ERR_EXPECTED_TOKENS:
            print_err_base(out, file_info, &err->base);
            print_expected_tokens_err(out, &err->expected_tokens_err);
            break;
    }
    fprintf(out, "\n");
}

static const char* get_single_macro_op_str(enum single_macro_op_type type) {
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

static const char* get_else_op_str(enum else_op_type type) {
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
                          size_t fail_file,
                          struct source_loc include_loc,
                          bool open_fail) {
    assert(fail_file != (size_t)-1);

    set_preproc_err(err, PREPROC_ERR_FILE_FAIL, include_loc);
    err->open_fail = open_fail;
    err->errno_state = errno;
    err->fail_file = fail_file;
    errno = 0;
}

void free_preproc_err(struct preproc_err* err) {
    assert(err);

    switch (err->type) {
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
        case PREPROC_ERR_FILE_FAIL:
        case PREPROC_ERR_MACRO_ARG_COUNT:
        case PREPROC_ERR_NONE:
        case PREPROC_ERR_UNTERMINATED_LIT:
        case PREPROC_ERR_UNTERMINATED_MACRO:
        case PREPROC_ERR_ARG_COUNT:
        case PREPROC_ERR_IFDEF_NOT_ID:
        case PREPROC_ERR_MISSING_IF:
        case PREPROC_ERR_INVALID_PREPROC_DIR:
        case PREPROC_ERR_ELIF_ELSE_AFTER_ELSE:
        case PREPROC_ERR_MISPLACED_PREPROC_TOKEN:
        case PREPROC_ERR_EMPTY_DEFINE:
        case PREPROC_ERR_DEFINE_NOT_ID:
        case PREPROC_ERR_EXPECTED_TOKENS:
            break;
    }
}

