#ifndef POSTFIX_EXPR_H
#define POSTFIX_EXPR_H

#include "ArgExprList.h"

#include "frontend/ast/Initializer.h"

typedef struct PrimaryExpr PrimaryExpr;
typedef struct Expr Expr;
typedef struct Identifier Identifier;

typedef enum {
    POSTFIX_INDEX,
    POSTFIX_BRACKET,
    POSTFIX_ACCESS,
    POSTFIX_PTR_ACCESS,
    POSTFIX_INC,
    POSTFIX_DEC,
} PostfixSuffixKind;

typedef struct {
    PostfixSuffixKind kind;
    union {
        Expr* index_expr;
        ArgExprList bracket_list;
        Identifier* identifier;
    };
} PostfixSuffix;

typedef struct PostfixExpr {
    bool is_primary;
    union {
        PrimaryExpr* primary;
        struct {
            AstNodeInfo info;
            TypeName* type_name;
            InitList init_list;
        };
    };
    size_t len;
    PostfixSuffix* suffixes;
} PostfixExpr;

PostfixExpr* parse_postfix_expr(ParserState* s);

/**
 *
 * @param s current state
 * @param type_name A type name that was already parsed by parse_unary_expr
 * @param start_bracket_loc Location of the bracket starting this expr
 *
 * @return A postfix_expr that uses the given type_name
 */
PostfixExpr* parse_postfix_expr_type_name(ParserState* s,
                                                  TypeName* type_name,
                                                  SourceLoc start_bracket_loc);

void PostfixExpr_free(PostfixExpr* p);

#include "PrimaryExpr.h"
#include "Expr.h"

#include "frontend/ast/Identifier.h"

#endif

