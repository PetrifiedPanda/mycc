#ifndef COMPOUND_STATEMENT_H
#define COMPOUND_STATEMENT_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

struct block_item;

struct compound_statement {
    struct ast_node_info info;
    size_t len;
    struct block_item* items;
};

struct compound_statement* parse_compound_statement(struct parser_state* s);

void free_compound_statement(struct compound_statement* s);

#include "block_item.h"

#endif
