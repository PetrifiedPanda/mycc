#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <stdbool.h>

#include "AstNodeInfo.h"

typedef struct Identifier {
    AstNodeInfo info;
    Str spelling;
} Identifier;

void init_identifier(Identifier* res,
                     const Str* spelling,
                     SourceLoc loc);
Identifier* create_identifier(const Str* spelling,
                                     SourceLoc loc);

void free_identifier_children(Identifier* i);

void free_identifier(Identifier* i);

#endif

