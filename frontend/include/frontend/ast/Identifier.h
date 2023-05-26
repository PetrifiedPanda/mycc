#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <stdbool.h>

#include "AstNodeInfo.h"

typedef struct Identifier {
    AstNodeInfo info;
    Str spelling;
} Identifier;

void Identifier_init(Identifier* res,
                     const Str* spelling,
                     SourceLoc loc);
Identifier* Identifier_create(const Str* spelling,
                                     SourceLoc loc);

void Identifier_free_children(Identifier* i);

void Identifier_free(Identifier* i);

#endif

