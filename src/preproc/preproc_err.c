#include "preproc/preproc_err.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

struct preproc_err create_preproc_err() {
    return (struct preproc_err){
        .type = PREPROC_ERR_NONE,
        .base.file = NULL,
    };
}

void set_preproc_err(struct preproc_err* err,
                     enum preproc_err_type type,
                     char* file,
                     struct source_location loc) {
    assert(err);
    assert(type != PREPROC_ERR_NONE);
    assert(err->type == PREPROC_ERR_NONE);

    err->type = type;
    err->base = (struct err_base){
        .file = file,
        .loc = loc,
    };
}

void set_preproc_err_copy(struct preproc_err* err,
                         enum preproc_err_type type,
                         const char* file,
                         struct source_location loc) {
    assert(err);
    assert(type != PREPROC_ERR_NONE);
    assert(err->type == PREPROC_ERR_NONE);
    assert(file);

    err->type = type;
    err->base.file = alloc_string_copy(file);
    err->base.loc = loc;
}

void print_preproc_err(struct preproc_err* err) {
    assert(err->type != PREPROC_ERR_NONE);

    switch (err->type) {
        case PREPROC_ERR_NONE:
            assert(false);
            break;
        case PREPROC_ERR_FILE_FAIL:
            if (err->base.file) {
                print_err_base(&err->base);
            }
            if (err->open_fail) {
                printf("Failed to open file %s", err->fail_file);
            } else {
                printf("Failed to close file %s", err->fail_file);
            }
            break;
        case PREPROC_ERR_UNTERMINATED_LIT:
            print_err_base(&err->base);
            printf("%s literal not properly terminated",
                   err->is_char_lit ? "Char" : "String");
            break;
        case PREPROC_ERR_INVALID_ID:
            print_err_base(&err->base);
            printf("Invalid identifier: %s", err->invalid_id);
            break;
        case PREPROC_ERR_MACRO_ARG_COUNT:
            print_err_base(&err->base);
            if (err->too_few_args) {
                printf("Too few arguments in function-like macro invocation: "
                       "Expected%s %zu arguments",
                       err->is_variadic ? " at least" : "",
                       err->expected_arg_count);
            } else {
                printf("Too many arguments in function like macro invocation: "
                       "Expected only %zu arguments",
                       err->expected_arg_count);
            }
            break;
        case PREPROC_ERR_UNTERMINATED_MACRO:
            print_err_base(&err->base);
            printf("Unterminated macro invocation");
            break;
    }
    printf("\n");
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

