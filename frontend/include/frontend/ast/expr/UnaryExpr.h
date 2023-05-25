#ifndef UNARY_EXPR_H
#define UNARY_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

#include "frontend/ast/AstNodeInfo.h"

typedef struct PostfixExpr PostfixExpr;
typedef struct CastExpr CastExpr;
typedef struct TypeName TypeName;

typedef enum {
    UNARY_POSTFIX,
    UNARY_ADDRESSOF,
    UNARY_DEREF,
    UNARY_PLUS,
    UNARY_MINUS,
    UNARY_BNOT,
    UNARY_NOT,
    UNARY_SIZEOF_TYPE,
    UNARY_ALIGNOF,
} UnaryExprKind;

typedef enum {
    UNARY_OP_INC,
    UNARY_OP_DEC,
    UNARY_OP_SIZEOF,
} UnaryExprOp;

typedef struct UnaryExpr {
    AstNodeInfo info;
    size_t len;
    UnaryExprOp* ops_before; 
    UnaryExprKind kind;
    union {
        PostfixExpr* postfix;
        CastExpr* cast_expr;
        TypeName* type_name;
    };
} UnaryExpr;

UnaryExpr* parse_unary_expr(ParserState* s);

/**
 *
 * @param s current state
 * @param ops_before array of len tokens
 * @param len length of ops_before
 * @param type_name the already parsed type_name, with which this starts
 * @param start_bracket_loc location of the bracket before the type name
 *
 * @return unary expression created with the given parameters
 *         NULL on fail This does not free any of the parameters
 */
UnaryExpr* parse_unary_expr_type_name(ParserState* s,
                                              UnaryExprOp* ops_before,
                                              size_t len,
                                              TypeName* type_name,
                                              SourceLoc start_bracket_loc);

void free_unary_expr_children(UnaryExpr* u);
void free_unary_expr(UnaryExpr* u);

#include "frontend/ast/TypeName.h"

#include "PostfixExpr.h"
#include "CastExpr.h"

#endif

