#ifndef PREPROC_ERR_H
#define PREPROC_ERR_H

#include <stdio.h>

#include "frontend/err_base.h"

enum preproc_err_type {
    PREPROC_ERR_NONE = 0,
    PREPROC_ERR_FILE_FAIL,
    PREPROC_ERR_UNTERMINATED_LIT,
    PREPROC_ERR_INVALID_ID,
    PREPROC_ERR_MACRO_ARG_COUNT,
    PREPROC_ERR_UNTERMINATED_MACRO,
};

struct preproc_err {
    enum preproc_err_type type;
    struct err_base base;
    union {
        struct {
            bool open_fail;
            char* fail_file;
        };
        bool is_char_lit;
        char* invalid_id;
        struct {
            size_t expected_arg_count;
            bool too_few_args;
            bool is_variadic;
        };
    };
};

struct preproc_err create_preproc_err(void);

void set_preproc_err(struct preproc_err* err,
                     enum preproc_err_type type,
                     struct source_loc* loc);

void set_preproc_err_copy(struct preproc_err* err,
                          enum preproc_err_type type,
                          const struct source_loc* loc);

void print_preproc_err(FILE* out, struct preproc_err* err);

void free_preproc_err(struct preproc_err* err);

#endif

