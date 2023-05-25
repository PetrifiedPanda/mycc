#ifndef DIRECT_DECLARATOR_H
#define DIRECT_DECLARATOR_H

#include <stdbool.h>
#include <stddef.h>

#include "ParamTypeList.h"
#include "IdentifierList.h"

#include "frontend/parser/ParserState.h"

typedef struct AssignExpr AssignExpr;
typedef struct ConstExpr ConstExpr;
typedef struct Declarator Declarator;
typedef struct Identifier Identifier;

typedef enum {
    ARR_OR_FUNC_ARRAY,
    ARR_OR_FUNC_FUN_PARAMS,
    ARR_OR_FUNC_FUN_OLD_PARAMS,
    ARR_OR_FUNC_FUN_EMPTY
} ArrOrFuncSuffixKind;

typedef struct {
    bool is_static;
    TypeQuals type_quals;
    bool is_asterisk; // if this is true arr_len should be NULL
    AssignExpr* arr_len;
} ArrSuffix;

typedef struct {
    AstNodeInfo info;
    ArrOrFuncSuffixKind kind;
    union {
        ArrSuffix arr_suffix;
        ParamTypeList fun_types;
        IdentifierList fun_params;
    };
} ArrOrFuncSuffix;

typedef struct DirectDeclarator {
    AstNodeInfo info;
    bool is_id;
    union {
        Identifier* id;
        Declarator* bracket_decl;
    };
    size_t len;
    ArrOrFuncSuffix* suffixes;
} DirectDeclarator;

/**
 * @param s current ParserState
 * @param res direct_decl already initialized except suffixes, will be freed on
 *            failure
 */
bool parse_arr_or_func_suffixes(ParserState* s, DirectDeclarator* res);

DirectDeclarator* parse_direct_declarator(ParserState* s);
DirectDeclarator* parse_direct_declarator_typedef(ParserState* s);

void free_direct_declarator(DirectDeclarator* d);

#include "Declarator.h"

#include "frontend/ast/Identifier.h"

#include "frontend/ast/expr/AssignExpr.h"
#include "frontend/ast/expr/ConstExpr.h"

#endif

