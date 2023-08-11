#ifndef MYCC_FRONTEND_AST_UNARY_EXPR_H
#define MYCC_FRONTEND_AST_UNARY_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

#include "frontend/ast/Initializer.h"
#include "frontend/ast/AstNodeInfo.h"
#include "frontend/ast/StringLiteralNode.h"

#include "Expr.h"
#include "GenericSel.h"

typedef enum {
    CONSTANT_ENUM,
    CONSTANT_VAL,
} ConstantKind;

typedef struct {
    AstNodeInfo info;
    ConstantKind kind;
} Constant;

typedef struct {
    bool is_func;
    union {
        StringLiteralNode lit;
        AstNodeInfo info;
    };
} StringConstant;

typedef struct Identifier Identifier;

typedef enum {
    PRIMARY_EXPR_IDENTIFIER,
    PRIMARY_EXPR_CONSTANT,
    PRIMARY_EXPR_STRING_LITERAL,
    PRIMARY_EXPR_BRACKET,
    PRIMARY_EXPR_GENERIC
} PrimaryExprKind;

typedef struct PrimaryExpr {
    PrimaryExprKind kind;
    union {
        Constant constant;
        StringConstant string;
        Identifier* identifier;
        struct {
            AstNodeInfo info;
            Expr bracket_expr;
        };
        GenericSel generic;
    };
} PrimaryExpr;

typedef struct {
    uint32_t len;
    AssignExpr* assign_exprs;
} ArgExprList;

typedef struct Identifier Identifier;
typedef struct TypeName TypeName;

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
        Expr index_expr;
        ArgExprList bracket_list;
        Identifier* identifier;
    };
} PostfixSuffix;

typedef struct PostfixExpr {
    bool is_primary;
    union {
        PrimaryExpr primary;
        struct {
            AstNodeInfo info;
            TypeName* type_name;
            InitList init_list;
        };
    };
    uint32_t len;
    PostfixSuffix* suffixes;
} PostfixExpr;

typedef struct TypeName TypeName;
typedef struct CastExpr CastExpr;

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
    uint32_t len;
    UnaryExprOp* ops_before;
    UnaryExprKind kind;
    union {
        PostfixExpr postfix;
        CastExpr* cast_expr;
        TypeName* type_name;
    };
} UnaryExpr;

bool parse_unary_expr_inplace(ParserState* s, UnaryExpr* res);

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
bool parse_unary_expr_type_name(ParserState* s,
                                UnaryExpr* res,
                                UnaryExprOp* ops_before,
                                uint32_t len,
                                TypeName* type_name,
                                uint32_t start_bracket_idx);

void PrimaryExpr_free_children(PrimaryExpr* e);

void ArgExprList_free(ArgExprList* l);

void PostfixExpr_free_children(PostfixExpr* p);

void UnaryExpr_free_children(UnaryExpr* u);

#include "frontend/ast/TypeName.h"

#endif

