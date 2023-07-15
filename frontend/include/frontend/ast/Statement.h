#ifndef MYCC_FRONTEND_AST_STATEMENT_H
#define MYCC_FRONTEND_AST_STATEMENT_H

#include "frontend/parser/ParserState.h"

#include "frontend/ast/declaration/Declaration.h"

#include "frontend/ast/expr/Expr.h"
#include "frontend/ast/expr/AssignExpr.h"

#include "AstNodeInfo.h"

typedef struct LabeledStatement LabeledStatement;
typedef struct CompoundStatement CompoundStatement;
typedef struct ExprStatement ExprStatement;
typedef struct SelectionStatement SelectionStatement;
typedef struct IterationStatement IterationStatement;
typedef struct JumpStatement JumpStatement;

typedef enum {
    STATEMENT_LABELED,
    STATEMENT_COMPOUND,
    STATEMENT_EXPRESSION,
    STATEMENT_SELECTION,
    STATEMENT_ITERATION,
    STATEMENT_JUMP
} StatementKind;

typedef struct Statement {
    StatementKind kind;
    union {
        LabeledStatement* labeled;
        CompoundStatement* comp;
        ExprStatement* expr;
        SelectionStatement* sel;
        IterationStatement* it;
        JumpStatement* jmp;
    };
} Statement;

typedef struct Identifier Identifier;

typedef enum {
    LABELED_STATEMENT_CASE,
    LABELED_STATEMENT_LABEL,
    LABELED_STATEMENT_DEFAULT,
} LabeledStatementKind;

typedef struct LabeledStatement {
    AstNodeInfo info;
    LabeledStatementKind kind;
    union {
        Identifier* label;
        ConstExpr case_expr;
    };
    Statement* stat;
} LabeledStatement;

typedef struct {
    bool is_decl;
    union {
        Declaration decl;
        Statement stat;
    };
} BlockItem;

typedef struct CompoundStatement {
    AstNodeInfo info;
    size_t len;
    BlockItem* items;
} CompoundStatement;

typedef struct ExprStatement {
    AstNodeInfo info;
    Expr expr;
} ExprStatement;

typedef struct SelectionStatement {
    AstNodeInfo info;
    bool is_if;
    Expr sel_expr;
    Statement* sel_stat;
    Statement* else_stat;
} SelectionStatement;

typedef struct {
    bool is_decl;
    union {
        Declaration init_decl;
        ExprStatement* init_expr;
    };
    ExprStatement* cond;
    Expr incr_expr; // when empty len == 0
} ForLoop;

typedef enum {
    ITERATION_STATEMENT_WHILE,
    ITERATION_STATEMENT_DO,
    ITERATION_STATEMENT_FOR,
} IterationStatementKind;

typedef struct IterationStatement {
    AstNodeInfo info;
    IterationStatementKind kind;
    Statement* loop_body;
    union {
        Expr while_cond;
        ForLoop for_loop;
    };
} IterationStatement;

typedef enum {
    JUMP_STATEMENT_GOTO,
    JUMP_STATEMENT_CONTINUE,
    JUMP_STATEMENT_BREAK,
    JUMP_STATEMENT_RETURN,
} JumpStatementKind;

typedef struct JumpStatement {
    AstNodeInfo info;
    JumpStatementKind kind;
    union {
        Identifier* goto_label;
        Expr ret_val;
    };
} JumpStatement;

bool parse_statement_inplace(ParserState* s, Statement* res);
Statement* parse_statement(ParserState* s);

bool parse_compound_statement_inplace(ParserState* s, CompoundStatement* res);

void Statement_free_children(Statement* s);
void Statement_free(Statement* s);

void LabeledStatement_free(LabeledStatement* s);

void CompoundStatement_free(CompoundStatement* s);
void CompoundStatement_free_children(CompoundStatement* s);

void ExprStatement_free(ExprStatement* s);

void SelectionStatement_free(SelectionStatement* s);

void IterationStatement_free(IterationStatement* s);

void JumpStatement_free(JumpStatement* s);

#endif

