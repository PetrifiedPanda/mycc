#ifndef TYPE_QUAL_H
#define TYPE_QUAL_H

#include <stdbool.h>

#include "token_type.h"

#include "parser/parser_state.h"

struct type_qual {
    enum token_type type;
};

struct type_qual parse_type_qual(struct parser_state* s);

#endif

