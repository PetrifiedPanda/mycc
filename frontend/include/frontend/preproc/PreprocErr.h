#ifndef PREPROC_ERR_H
#define PREPROC_ERR_H

#include <stdio.h>

#include "util/Str.h"

#include "frontend/ErrBase.h"
#include "frontend/ExpectedTokensErr.h"

#include "num_parse.h"

typedef enum {
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
} PreprocErrKind;

typedef enum {
    ELSE_OP_ELIF,
    ELSE_OP_ELSE,
    ELSE_OP_ENDIF,
} ElseOpKind;

typedef enum {
    SINGLE_MACRO_OP_IFDEF,
    SINGLE_MACRO_OP_IFNDEF,
    SINGLE_MACRO_OP_UNDEF,
} SingleMacroOpKind;

typedef struct {
    PreprocErrKind kind;
    ErrBase base;
    union {
        struct {
            int errno_state;
            Str fail_filename;
        };
        bool is_char_lit;
        Str invalid_id;
        Str invalid_num;
        struct {
            size_t expected_arg_count;
            bool too_few_args;
            bool is_variadic;
        };
        SourceLoc unterminated_cond_loc;
        struct {
            SingleMacroOpKind count_dir_kind;
            bool count_empty;
        };
        struct {
            SingleMacroOpKind not_identifier_op;
            TokenKind not_identifier_got;
        };
        ElseOpKind missing_if_op;
        struct {
            ElseOpKind elif_after_else_op;
            SourceLoc prev_else_loc;
        };
        TokenKind misplaced_preproc_tok;
        struct {
            Str constant_spell;
            union {
                IntConstErr int_const_err;
                FloatConstErr float_const_err;
                CharConstErr char_const_err;
            };
        };
        TokenKind type_instead_of_identifier;
        ExpectedTokensErr expected_tokens_err;
        Str duplicate_arg_name;
    };
} PreprocErr;

PreprocErr create_preproc_err(void);

void set_preproc_err(PreprocErr* err, PreprocErrKind kind, SourceLoc loc);

void print_preproc_err(FILE* out, const FileInfo* file_info, PreprocErr* err);

void set_preproc_file_err(PreprocErr* err, const Str* fail_filename, SourceLoc include_loc);

void free_preproc_err(PreprocErr* err);

#endif

