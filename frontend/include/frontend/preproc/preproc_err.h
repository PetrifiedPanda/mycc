#ifndef PREPROC_ERR_H
#define PREPROC_ERR_H

#include <stdio.h>

#include "util/str.h"

#include "frontend/err_base.h"
#include "frontend/expected_tokens_err.h"

#include "frontend/preproc/num_parse.h"

enum preproc_err_kind {
    PREPROC_ERR_NONE = 0,
    PREPROC_ERR_OPEN_FILE,
    PREPROC_ERR_UNTERMINATED_LIT,
    PREPROC_ERR_INVALID_ID,
    PREPROC_ERR_INVALID_NUMBER,
    PREPROC_ERR_MACRO_ARG_COUNT,
    PREPROC_ERR_UNTERMINATED_MACRO,
    PREPROC_ERR_UNTERMINATED_COND,
    PREPROC_ERR_ARG_COUNT,
    PREPROC_ERR_IFDEF_NOT_ID,
    PREPROC_ERR_MISSING_IF,
    PREPROC_ERR_INVALID_PREPROC_DIR,
    PREPROC_ERR_ELIF_ELSE_AFTER_ELSE,
    PREPROC_ERR_MISPLACED_PREPROC_TOKEN,
    PREPROC_ERR_INT_CONST,
    PREPROC_ERR_FLOAT_CONST,
    PREPROC_ERR_CHAR_CONST,
    PREPROC_ERR_EMPTY_DEFINE,
    PREPROC_ERR_DEFINE_NOT_ID,
    PREPROC_ERR_EXPECTED_TOKENS,
    PREPROC_ERR_DUPLICATE_MACRO_PARAM,
    PREPROC_ERR_INVALID_BACKSLASH,
    PREPROC_ERR_INCLUDE_NUM_ARGS,
    PREPROC_ERR_INCLUDE_NOT_STRING_LITERAL,
};

enum else_op_kind {
    ELSE_OP_ELIF,
    ELSE_OP_ELSE,
    ELSE_OP_ENDIF,
};

enum single_macro_op_kind {
    SINGLE_MACRO_OP_IFDEF,
    SINGLE_MACRO_OP_IFNDEF,
    SINGLE_MACRO_OP_UNDEF,
};

struct preproc_err {
    enum preproc_err_kind kind;
    struct err_base base;
    union {
        struct {
            int errno_state;
            struct str fail_filename;
        };
        bool is_char_lit;
        struct str invalid_id;
        struct str invalid_num;
        struct {
            size_t expected_arg_count;
            bool too_few_args;
            bool is_variadic;
        };
        struct source_loc unterminated_cond_loc;
        struct {
            enum single_macro_op_kind count_dir_kind;
            bool count_empty;
        };
        struct {
            enum single_macro_op_kind not_identifier_op;
            enum token_kind not_identifier_got;
        };
        enum else_op_kind missing_if_op;
        struct {
            enum else_op_kind elif_after_else_op;
            struct source_loc prev_else_loc;
        };
        enum token_kind misplaced_preproc_tok;
        struct {
            struct str constant_spell;
            union {
                struct int_const_err int_const_err;
                struct float_const_err float_const_err;
                struct char_const_err char_const_err;
            };
        };
        enum token_kind type_instead_of_identifier;
        struct expected_tokens_err expected_tokens_err;
        struct str duplicate_arg_name;
    };
};

struct preproc_err create_preproc_err(void);

void set_preproc_err(struct preproc_err* err,
                     enum preproc_err_kind kind,
                     struct source_loc loc);

void print_preproc_err(FILE* out,
                       const struct file_info* file_info,
                       struct preproc_err* err);

void set_preproc_file_err(struct preproc_err* err,
                          const struct str* fail_filename,
                          struct source_loc include_loc);

void free_preproc_err(struct preproc_err* err);

#endif

