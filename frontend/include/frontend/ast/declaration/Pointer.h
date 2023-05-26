#ifndef POINTER_H
#define POINTER_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

#include "frontend/ast/AstNodeInfo.h"

typedef struct TypeQuals TypeQuals;

typedef struct Pointer {
    AstNodeInfo info;
    size_t num_indirs;
    TypeQuals* quals_after_ptr;
} Pointer;

Pointer* parse_pointer(ParserState* s);

void Pointer_free(Pointer* p);

#include "TypeQuals.h"

#endif

