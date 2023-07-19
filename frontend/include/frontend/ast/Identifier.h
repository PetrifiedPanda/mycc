#ifndef MYCC_FRONTEND_AST_IDENTIFIER_H
#define MYCC_FRONTEND_AST_IDENTIFIER_H

#include <stdbool.h>

#include "AstNodeInfo.h"

typedef struct Identifier {
    AstNodeInfo info;
    StrBuf spelling;
} Identifier;

void Identifier_init(Identifier* res,
                     const StrBuf* spelling,
                     SourceLoc loc);
Identifier* Identifier_create(const StrBuf* spelling,
                                     SourceLoc loc);

void Identifier_free_children(Identifier* i);

void Identifier_free(Identifier* i);

#endif

