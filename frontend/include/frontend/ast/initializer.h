#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <stdbool.h>

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

struct designator;

struct designator_list {
    size_t len;
    struct designator* designators;
};

struct designation_init;

struct init_list {
    size_t len;
    struct designation_init* inits;
};

struct assign_expr;

struct initializer {
    struct ast_node_info info;
    bool is_assign;
    union {
        struct assign_expr* assign;
        struct init_list init_list;
    };
};

struct const_expr;
struct identifier;

struct designator {
    struct ast_node_info info;
    bool is_index;
    union {
        struct const_expr* arr_index;
        struct identifier* identifier;
    };
};

struct designation {
    struct designator_list designators;
};

struct designation_init {
    struct designation designation;
    struct initializer init;
};

struct initializer* parse_initializer(struct parser_state* s);
bool parse_init_list(struct parser_state* s, struct init_list* res);

struct designation* parse_designation(struct parser_state* s);

struct designation create_invalid_designation(void);

void free_initializer_children(struct initializer* i);
void free_initializer(struct initializer* i);

void free_designator_children(struct designator* d);

void free_designator_list(struct designator_list* l);

bool is_valid_designation(const struct designation* d);

void free_designation(struct designation* d);
void free_designation_children(struct designation* d);

void free_init_list_children(struct init_list* l);

#include "frontend/ast/expr/assign_expr.h"
#include "frontend/ast/expr/const_expr.h"

#include "frontend/ast/identifier.h"

#endif
