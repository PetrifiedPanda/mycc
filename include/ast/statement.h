#ifndef STATEMENT_H
#define STATEMENT_H

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
} StatementType;

typedef struct Statement {
    StatementType type;
    union {
        LabeledStatement* labeled;
        CompoundStatement* comp;
        ExprStatement* expr;
        SelectionStatement* sel;
        IterationStatement* it;
        JumpStatement* jmp;
    };
} Statement;

Statement* create_statement_labeled(LabeledStatement* labeled);
Statement* create_statement_compound(CompoundStatement* comp);
Statement* create_statement_select(SelectionStatement* sel);
Statement* create_statement_iter(IterationStatement* it);
Statement* create_statement_jump(JumpStatement* jmp);

void free_statement_children(Statement* s);

void free_statement(Statement* s);

#include "ast/labeled_statement.h"
#include "ast/compound_statement.h"
#include "ast/expr_statement.h"
#include "ast/selection_statement.h"
#include "ast/iteration_statement.h"
#include "ast/jump_statement.h"

#endif
