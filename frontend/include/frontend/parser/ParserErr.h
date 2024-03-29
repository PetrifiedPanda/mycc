#ifndef MYCC_FRONTEND_PARSER_PARSER_ERR_H
#define MYCC_FRONTEND_PARSER_PARSER_ERR_H

#include "util/File.h"

#include "frontend/ErrBase.h"
#include "frontend/ExpectedTokensErr.h"
#include "frontend/Token.h"

typedef enum {
    PARSER_ERR_NONE = 0,
    PARSER_ERR_EXPECTED_TOKENS,
    PARSER_ERR_REDEFINED_SYMBOL,
    PARSER_ERR_ARR_DOUBLE_STATIC,
    PARSER_ERR_ARR_STATIC_NO_LEN,
    PARSER_ERR_ARR_STATIC_ASTERISK,
    PARSER_ERR_TYPEDEF_INIT,
    PARSER_ERR_TYPEDEF_FUNC_DEF,
    PARSER_ERR_TYPEDEF_PARAM_DECL,
    PARSER_ERR_TYPEDEF_STRUCT,
    PARSER_ERR_EMPTY_STRUCT_DECLARATOR,
    PARSER_ERR_INCOMPATIBLE_TYPE_SPECS,
    PARSER_ERR_TOO_MUCH_LONG,
    PARSER_ERR_DISALLOWED_TYPE_QUALS,
    PARSER_ERR_EXPECTED_TYPEDEF_NAME,
    PARSER_ERR_EMPTY_DIRECT_ABS_DECL,
    PARSER_ERR_TYPEDEF_WITHOUT_DECLARATOR,
} ParserErrKind;

typedef struct ParserErr {
    ParserErrKind kind;
    uint32_t err_token_idx;
    union {
        ExpectedTokensErr expected_tokens_err; 
        struct { // redefined symbol
            bool was_typedef_name;
            uint32_t prev_def_idx;
        };
        struct { // incompatible type specs
            TokenKind type_spec, prev_type_spec;
        };
        // disallowed type specs
        TokenKind incompatible_type;
    };
} ParserErr;

ParserErr ParserErr_create(void);

void ParserErr_set(ParserErr* err, ParserErrKind kind, uint32_t loc);

void ParserErr_print(File out, const FileInfo* file_info, const TokenArr* tokens, const ParserErr* err);

#endif

