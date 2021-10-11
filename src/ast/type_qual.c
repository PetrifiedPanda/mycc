#include "ast/type_qual.h"

#include <stdlib.h>
#include <assert.h>

TypeQual create_type_qual(TokenType type) {
    assert(type == VOLATILE || type == CONST);
    TypeQual res;
    res.is_const = type == CONST;
    return res;
}
