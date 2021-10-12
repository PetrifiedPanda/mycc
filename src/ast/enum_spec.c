#include "ast/enum_spec.h"

#include <stdlib.h>
#include <assert.h>

EnumSpec* create_enum_spec(char* identifier, EnumList* enum_list) {
    assert(identifier || enum_list); 
    EnumSpec* res = malloc(sizeof(EnumSpec));
    if (res) {
        res->identifier = identifier;
        res->enum_list = enum_list;
    }
    return res;
}

static void free_children(EnumSpec* s) {
    free(s->identifier);
    if (s->enum_list) {
        free_enum_list(s->enum_list);
    }
}

void free_enum_spec(EnumSpec* s) {
    free_children(s);
    free(s);
}
