#ifndef TYPE_QUAL_H
#define TYPE_QUAL_H

#include <stdbool.h>

#include "token_type.h"

typedef struct TypeQual {
    bool is_const; // either volatile or const    
} TypeQual;

TypeQual create_type_qual(TokenType type);

#endif

