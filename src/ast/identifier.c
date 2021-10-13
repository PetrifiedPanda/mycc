#include "ast/identifier.h"

#include <stdlib.h>

void free_identifier(Identifier* i) {
    free(i->spelling);
}
