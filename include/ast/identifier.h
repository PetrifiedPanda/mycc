#ifndef IDENTIFIER_H
#define IDENTIFIER_H

struct identifier {
    char* spelling;
};

void create_identifier_inplace(struct identifier* res, char* spelling);
struct identifier* create_identifier(char* spelling);

void free_identifier_children(struct identifier* i);

void free_identifier(struct identifier* i);

#endif
