#ifndef ENUM_SPEC_H
#define ENUM_SPEC_H

#include "ast/enum_list.h"

typedef struct EnumSpec {
    char* identifier;
    EnumList enum_list;
} EnumSpec;

EnumSpec* create_enum_spec(char* identifier, EnumList enum_list);

void free_enum_spec(EnumSpec* s);

#endif
