#include "frontend/preproc/preproc_err.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "frontend/err_base.h"

#include "util/mem.h"
#include "util/annotations.h"

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
            const char* fail_path = file_info->paths[err->fail_file];
            if (err->open_fail) {
                fprintf(out, "Failed to open file %s", fail_path);
            } else {
                fprintf(out, "Failed to close file %s", fail_path);
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
            fprintf(out, "Invalid identifier: %s", err->invalid_id);
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
            fprintf(out, "Unterminated macro invocation");
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
        case PREPROC_ERR_NOT_IDENTIFIER: {
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
            const char*
                prev_else_file = file_info->paths[err->prev_else_loc.file_idx];
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
        }
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

void free_preproc_err(struct preproc_err* err) {
    assert(err);

    switch (err->type) {
        case PREPROC_ERR_INVALID_ID:
            free(err->invalid_id);
            break;
        case PREPROC_ERR_FILE_FAIL:
        case PREPROC_ERR_MACRO_ARG_COUNT:
        case PREPROC_ERR_NONE:
        case PREPROC_ERR_UNTERMINATED_LIT:
        case PREPROC_ERR_UNTERMINATED_MACRO:
        case PREPROC_ERR_ARG_COUNT:
        case PREPROC_ERR_NOT_IDENTIFIER:
        case PREPROC_ERR_MISSING_IF:
        case PREPROC_ERR_INVALID_PREPROC_DIR:
        case PREPROC_ERR_ELIF_ELSE_AFTER_ELSE:
            break;
    }
}

