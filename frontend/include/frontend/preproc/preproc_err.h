#ifndef PREPROC_ERR_H
#define PREPROC_ERR_H

#include <stdio.h>

#include "util/str.h"

#include "frontend/err_base.h"

#include "frontend/preproc/num_parse.h"

enum preproc_err_type {
    PREPROC_ERR_NONE = 0,
    PREPROC_ERR_FILE_FAIL,
    PREPROC_ERR_UNTERMINATED_LIT,
    PREPROC_ERR_INVALID_ID,
    PREPROC_ERR_INVALID_NUMBER,
    PREPROC_ERR_MACRO_ARG_COUNT,
    PREPROC_ERR_UNTERMINATED_MACRO,
    PREPROC_ERR_ARG_COUNT,
    PREPROC_ERR_NOT_IDENTIFIER,
    PREPROC_ERR_MISSING_IF,
    PREPROC_ERR_INVALID_PREPROC_DIR,
    PREPROC_ERR_ELIF_ELSE_AFTER_ELSE,
    PREPROC_ERR_MISPLACED_PREPROC_TOKEN,
    PREPROC_ERR_INT_CONST,
    PREPROC_ERR_FLOAT_CONST,
    PREPROC_ERR_CHAR_CONST,
};

enum else_op_type {
    ELSE_OP_ELIF,
    ELSE_OP_ELSE,
    ELSE_OP_ENDIF,
};

enum single_macro_op_type {
    SINGLE_MACRO_OP_IFDEF,
    SINGLE_MACRO_OP_IFNDEF,
    SINGLE_MACRO_OP_UNDEF,
};

struct preproc_err {
    enum preproc_err_type type;
    struct err_base base;
    union {
        struct {
            bool open_fail;
            size_t fail_file;
        };
        bool is_char_lit;
        struct str invalid_id;
        struct str invalid_num;
        struct {
            size_t expected_arg_count;
            bool too_few_args;
            bool is_variadic;
        };
        struct {
            enum single_macro_op_type count_dir_type;
            bool count_empty;
        };
        struct {
            enum single_macro_op_type not_identifier_op;
            enum token_type not_identifier_got;
        };
        enum else_op_type missing_if_op;
        struct {
            enum else_op_type elif_after_else_op;
            struct source_loc prev_else_loc;
        };
        enum token_type misplaced_preproc_tok;
        struct {
            struct str constant_spell;
            union {
                struct int_const_err int_const_err;
                struct float_const_err float_const_err;
                struct char_const_err char_const_err;
            };
        };
    };
};

struct preproc_err create_preproc_err(void);

void set_preproc_err(struct preproc_err* err,
                     enum preproc_err_type type,
                     struct source_loc loc);

void print_preproc_err(FILE* out,
                       const struct file_info* file_info,
                       struct preproc_err* err);

void free_preproc_err(struct preproc_err* err);

#endif

