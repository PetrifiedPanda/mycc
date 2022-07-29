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
        .base.loc.file = NULL,
    };
}

void set_preproc_err(struct preproc_err* err,
                     enum preproc_err_type type,
                     struct source_loc* loc) {
    assert(err);
    assert(type != PREPROC_ERR_NONE);
    assert(err->type == PREPROC_ERR_NONE); 

    err->type = type;
    err->base = create_err_base(loc);
}

void set_preproc_err_copy(struct preproc_err* err,
                          enum preproc_err_type type,
                          const struct source_loc* loc) {
    assert(err);
    assert(type != PREPROC_ERR_NONE);
    assert(err->type == PREPROC_ERR_NONE);
    assert(loc);
    assert(loc->file);

    err->type = type;
    err->base = create_err_base_copy(loc);
}

void print_preproc_err(FILE* out, struct preproc_err* err) {
    assert(err->type != PREPROC_ERR_NONE);

    switch (err->type) {
        case PREPROC_ERR_NONE:
            UNREACHABLE();
            break;
        case PREPROC_ERR_FILE_FAIL:
            if (err->base.loc.file) {
                print_err_base(out, &err->base);
            }
            if (err->open_fail) {
                fprintf(out, "Failed to open file %s", err->fail_file);
            } else {
                fprintf(out, "Failed to close file %s", err->fail_file);
            }
            break;
        case PREPROC_ERR_UNTERMINATED_LIT:
            print_err_base(out, &err->base);
            fprintf(out, "%s literal not properly terminated",
                   err->is_char_lit ? "Char" : "String");
            break;
        case PREPROC_ERR_INVALID_ID:
            print_err_base(out, &err->base);
            fprintf(out, "Invalid identifier: %s", err->invalid_id);
            break;
        case PREPROC_ERR_MACRO_ARG_COUNT:
            print_err_base(out, &err->base);
            if (err->too_few_args) {
                fprintf(out, "Too few arguments in function-like macro invocation: "
                       "Expected%s %zu arguments",
                       err->is_variadic ? " at least" : "",
                       err->expected_arg_count);
            } else {
                fprintf(out, "Too many arguments in function like macro invocation: "
                       "Expected only %zu arguments",
                       err->expected_arg_count);
            }
            break;
        case PREPROC_ERR_UNTERMINATED_MACRO:
            print_err_base(out, &err->base);
            fprintf(out, "Unterminated macro invocation");
            break;
    }
    fprintf(out, "\n");
}

void free_preproc_err(struct preproc_err* err) {
    assert(err);

    free_err_base(&err->base);
    switch (err->type) {
        case PREPROC_ERR_FILE_FAIL:
            free(err->fail_file);
            break;
        case PREPROC_ERR_INVALID_ID:
            free(err->invalid_id);
            break;
        case PREPROC_ERR_MACRO_ARG_COUNT:
        case PREPROC_ERR_NONE:
        case PREPROC_ERR_UNTERMINATED_LIT:
        case PREPROC_ERR_UNTERMINATED_MACRO:
            break;
    }
}

