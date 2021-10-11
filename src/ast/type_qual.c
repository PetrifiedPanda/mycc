#include "ast/type_qual.h"

#include <stdlib.h>

static void free_children(TypeQual* t) {

}

void free_type_qual(TypeQual* t) {
    free_children(t);
    free(t);
}