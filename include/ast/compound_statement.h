#ifndef COMPOUND_STATEMENT_H
#define COMPOUND_STATEMENT_H

typedef struct StatementList StatementList;
typedef struct DeclarationList DeclarationList;

typedef struct CompoundStatement {
    DeclarationList* decl_list;
    StatementList* stat_list;
} CompoundStatement;

CompoundStatement* create_compound_statement(DeclarationList* decl_list, StatementList* stat_list);

void free_compound_statement(CompoundStatement* s);

#include "ast/statement_list.h"
#include "ast/declaration_list.h"

#endif
