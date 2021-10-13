#ifndef IDENTIFIER_H
#define IDENTIFIER_H

// TODO: figure out how to use this

typedef struct {
    char* spelling;
} Identifier;

void free_identifier(Identifier* i);

#endif
