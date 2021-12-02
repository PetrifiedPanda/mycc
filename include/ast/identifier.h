#ifndef IDENTIFIER_H
#define IDENTIFIER_H

typedef struct Identifier {
    char* spelling;
} Identifier;

Identifier* create_identifier(char* spelling);

void free_identifier(Identifier* i);

#endif
