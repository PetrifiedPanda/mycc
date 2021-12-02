#ifndef LABELED_STATEMENT_H
#define LABELED_STATEMENT_H

#include "token_type.h"

typedef struct ConstExpr ConstExpr;
typedef struct Statement Statement;
typedef struct Identifier Identifier;

typedef struct LabeledStatement {
    TokenType type;
    union {
        Identifier* identifier;
        ConstExpr* case_expr;
    };
    Statement* stat;
} LabeledStatement;

LabeledStatement* create_labeled_statement_goto(Identifier* identifier, Statement* stat);
LabeledStatement* create_labeled_statement_case(ConstExpr* case_expr, Statement* stat);
LabeledStatement* craete_labeled_statement_default(Statement* stat);

void free_labeled_statement(LabeledStatement* s);

#include "ast/const_expr.h"
#include "ast/statement.h"
#include "ast/identifier.h"

#endif

