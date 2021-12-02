#ifndef ENUM_SPEC_H
#define ENUM_SPEC_H

#include "ast/enum_list.h"

typedef struct Identifier Identifier;

typedef struct EnumSpec {
    Identifier* identifier;
    EnumList enum_list;
} EnumSpec;

EnumSpec* create_enum_spec(Identifier* identifier, EnumList enum_list);

void free_enum_spec(EnumSpec* s);

#include "ast/identifier.h"

#endif

