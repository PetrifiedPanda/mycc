#ifndef TYPE_QUAL_H
#define TYPE_QUAL_H

#include <stdbool.h>

#include "token_type.h"

struct type_qual {
    bool is_const; // either volatile or const    
};

struct type_qual create_type_qual(enum token_type type);

#endif

