#include "ast/enum_spec.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct enum_spec* create_enum_spec(struct identifier* identifier, struct enum_list enum_list) {
    assert(identifier || enum_list.len > 0); 
    struct enum_spec* res = xmalloc(sizeof(struct enum_spec));
    res->identifier = identifier;
    res->enum_list = enum_list;
    
    return res;
}

static void free_children(struct enum_spec* s) {
    free_identifier(s->identifier);
    free_enum_list(&s->enum_list);
}

void free_enum_spec(struct enum_spec* s) {
    free_children(s);
    free(s);
}

