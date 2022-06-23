#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <stdbool.h>

struct identifier {
    char* spelling;
};

void init_identifier(struct identifier* res, char* spelling);
struct identifier* create_identifier(char* spelling);

void free_identifier_children(struct identifier* i);

void free_identifier(struct identifier* i);

struct ast_visitor;

bool visit_identifier(struct ast_visitor* visitor, struct identifier* i);

#endif

