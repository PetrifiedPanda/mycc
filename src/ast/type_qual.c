#include "ast/type_qual.h"

#include <stdlib.h>
#include <assert.h>

struct type_qual create_type_qual(enum token_type type) {
    assert(type == VOLATILE || type == CONST);
    struct type_qual res;
    res.is_const = type == CONST;
    return res;
}

