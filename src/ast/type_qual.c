#include "ast/type_qual.h"

#include <stdlib.h>
#include <assert.h>

struct type_qual create_type_qual(enum token_type type) {
    assert(type == VOLATILE || type == CONST || type == RESTRICT || type == ATOMIC);
    struct type_qual res;
    res.type = type;
    return res;
}

