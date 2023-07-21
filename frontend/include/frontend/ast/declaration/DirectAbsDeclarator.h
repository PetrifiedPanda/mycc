#ifndef MYCC_FRONTEND_DECLARATION_DIRECT_ABSTRACT_DECLARATOR_H
#define MYCC_FRONTEND_DECLARATION_DIRECT_ABSTRACT_DECLARATOR_H

#include <stdbool.h>
#include <stddef.h>

#include "ParamTypeList.h"

#include "frontend/parser/ParserState.h"

#include "frontend/ast/AstNodeInfo.h"

#include "frontend/ast/declaration/TypeQuals.h"

typedef struct AbsDeclarator AbsDeclarator;
typedef struct AssignExpr AssignExpr;

typedef enum {
    ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY, // either [] or [*]
    ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN,
    ABS_ARR_OR_FUNC_SUFFIX_FUNC
} AbsArrOrFuncSuffixKind;

typedef struct {
    AstNodeInfo info;
    AbsArrOrFuncSuffixKind kind;
    union {
        bool has_asterisk;
        struct {
            bool is_static; // only if assign != NULL
            TypeQuals type_quals;
            AssignExpr* assign;
        };
        ParamTypeList func_types;
    };
} AbsArrOrFuncSuffix;

typedef struct DirectAbsDeclarator {
    AstNodeInfo info;
    AbsDeclarator* bracket_decl;

    uint32_t len;
    AbsArrOrFuncSuffix* following_suffixes;
} DirectAbsDeclarator;

/**
 * @param s current parser_state
 * @param res direct_abs_declarator that is fully initialized except for the
 *            suffixes, will be freed on failure
 */
bool parse_abs_arr_or_func_suffixes(ParserState* s, DirectAbsDeclarator* res);

DirectAbsDeclarator* parse_direct_abs_declarator(ParserState* s);

void DirectAbsDeclarator_free(DirectAbsDeclarator* d);

#endif
