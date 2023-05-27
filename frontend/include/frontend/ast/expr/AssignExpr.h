#ifndef ASSIGN_EXPR_H
#define ASSIGN_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

#include "Expr.h"
#include "UnaryExpr.h"

typedef struct TypeName TypeName;

typedef struct CastExpr {
    AstNodeInfo info;
    size_t len;
    TypeName* type_names;
    UnaryExpr rhs;
} CastExpr;

typedef enum {
    MUL_EXPR_MUL,
    MUL_EXPR_DIV,
    MUL_EXPR_MOD,
} MulExprOp;

typedef struct {
    MulExprOp op;
    CastExpr rhs;
} CastExprAndOp;

typedef struct MulExpr {
    CastExpr lhs;
    size_t len;
    CastExprAndOp* mul_chain;
} MulExpr;

typedef enum {
    ADD_EXPR_ADD,
    ADD_EXPR_SUB,
} AddExprOp;

typedef struct {
    AddExprOp op;
    MulExpr rhs;
} MulExprAndOp;

typedef struct AddExpr {
    MulExpr lhs;
    size_t len;
    MulExprAndOp* add_chain;
} AddExpr;

typedef enum {
    SHIFT_EXPR_LEFT,
    SHIFT_EXPR_RIGHT,
} ShiftExprOp;

typedef struct {
    ShiftExprOp op;
    AddExpr rhs;
} AddExprAndOp;

typedef struct ShiftExpr {
    AddExpr lhs;
    size_t len;
    AddExprAndOp* shift_chain;
} ShiftExpr;

typedef enum {
    REL_EXPR_LT,
    REL_EXPR_GT,
    REL_EXPR_LE,
    REL_EXPR_GE,
} RelExprOp;

typedef struct {
    RelExprOp op;
    ShiftExpr rhs;
} ShiftExprAndOp;

typedef struct RelExpr {
    ShiftExpr lhs;
    size_t len;
    ShiftExprAndOp* rel_chain;
} RelExpr;

typedef enum {
    EQ_EXPR_EQ,
    EQ_EXPR_NE,
} EqExprOp;

typedef struct {
    EqExprOp op;
    RelExpr rhs;
} RelExprAndOp;

typedef struct EqExpr {
    RelExpr lhs;
    size_t len;
    RelExprAndOp* eq_chain;
} EqExpr;

typedef struct AndExpr {
    size_t len;
    EqExpr* eq_exprs;
} AndExpr;

typedef struct XorExpr {
    size_t len;
    AndExpr* and_exprs;
} XorExpr;

typedef struct OrExpr {
    size_t len;
    XorExpr* xor_exprs;
} OrExpr;

typedef struct LogAndExpr {
    size_t len;
    OrExpr* or_exprs;
} LogAndExpr;

typedef struct LogOrExpr {
    size_t len;
    LogAndExpr* log_ands;
} LogOrExpr;

typedef struct {
    LogOrExpr log_or;
    Expr expr;
} LogOrAndExpr;

typedef struct CondExpr {
    size_t len;
    LogOrAndExpr* conditionals;
    LogOrExpr last_else;
} CondExpr;

typedef struct ConstExpr {
    CondExpr expr;
} ConstExpr;

typedef enum {
    ASSIGN_EXPR_ASSIGN,
    ASSIGN_EXPR_MUL,
    ASSIGN_EXPR_DIV,
    ASSIGN_EXPR_MOD,
    ASSIGN_EXPR_ADD,
    ASSIGN_EXPR_SUB,
    ASSIGN_EXPR_LSHIFT,
    ASSIGN_EXPR_RSHIFT,
    ASSIGN_EXPR_AND,
    ASSIGN_EXPR_XOR,
    ASSIGN_EXPR_OR,
} AssignExprOp;

typedef struct {
    UnaryExpr unary;
    AssignExprOp op;
} UnaryAndOp;

typedef struct AssignExpr {
    size_t len;
    UnaryAndOp* assign_chain;
    CondExpr value;
} AssignExpr;

CastExpr* parse_cast_expr(ParserState* s);

ConstExpr* parse_const_expr(ParserState* s);

bool parse_assign_expr_inplace(ParserState* s, AssignExpr* res);
AssignExpr* parse_assign_expr(ParserState* s);

void CastExpr_free_children(CastExpr* e);
void CastExpr_free(CastExpr* e);

void MulExpr_free_children(MulExpr* e);

void AddExpr_free_children(AddExpr* e);

void ShiftExpr_free_children(ShiftExpr* e);

void RelExpr_free_children(RelExpr* e);

void EqExpr_free_children(EqExpr* e);

void AndExpr_free_children(AndExpr* e);

void XorExpr_free_children(XorExpr* e);

void OrExpr_free_children(OrExpr* e);

void LogAndExpr_free_children(LogAndExpr* e);

void LogOrExpr_free_children(LogOrExpr* e);

void CondExpr_free_children(CondExpr* e);

void ConstExpr_free(ConstExpr* e);

void AssignExpr_free_children(AssignExpr* e);
void AssignExpr_free(AssignExpr* e);

#endif

