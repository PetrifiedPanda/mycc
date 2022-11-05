#ifndef INIT_DECLARATOR_LIST_H
#define INIT_DECLARATOR_LIST_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

struct init_declarator;

struct init_declarator_list {
    size_t len;
    struct init_declarator* decls;
};

/**
 *
 * @param s The current state
 * @param first_decl A heap allocated init declarator
 * @return struct init_declarator_list A list parsed with first_decl as the
 * first element in the list
 */
bool parse_init_declarator_list_first(struct parser_state* s,
                                      struct init_declarator_list* res,
                                      struct init_declarator* first_decl);

bool parse_init_declarator_list(struct parser_state* s,
                                struct init_declarator_list* res);

bool parse_init_declarator_list_typedef_first(
    struct parser_state* s,
    struct init_declarator_list* res,
    struct init_declarator* first_decl);

bool parse_init_declarator_list_typedef(struct parser_state* s,
                                        struct init_declarator_list* res);

void free_init_declarator_list(struct init_declarator_list* l);

#include "frontend/ast/declaration/init_declarator.h"

#endif

