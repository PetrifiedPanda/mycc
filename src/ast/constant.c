#include "ast/constant.h"

#include <stdlib.h>

Constant create_constant(bool is_float, char* spelling) {
    return (Constant){is_float, spelling};
}

void free_constant(Constant* c) {
    free(c->spelling);
}
