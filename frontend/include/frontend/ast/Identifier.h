#ifndef MYCC_FRONTEND_AST_IDENTIFIER_H
#define MYCC_FRONTEND_AST_IDENTIFIER_H

#include <stdbool.h>

#include "AstNodeInfo.h"

typedef struct Identifier {
    AstNodeInfo info;
} Identifier;

void Identifier_init(Identifier* res,
                     uint32_t token_idx);
Identifier* Identifier_create(uint32_t token_idx);

void Identifier_free(Identifier* i);

#endif

