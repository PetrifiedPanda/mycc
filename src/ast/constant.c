#include "ast/constant.h"

#include <stdlib.h>

Constant create_constant(char* spelling) {
    return (Constant){spelling};
}

void free_constant(Constant* c) {
    free(c->spelling);
}
