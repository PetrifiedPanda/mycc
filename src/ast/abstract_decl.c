#include "ast/abstract_decl.h"

#include <stdlib.h>

static void free_children(AbstractDecl* d) {

}

void free_abstract_decl(AbstractDecl* d) {
    free_children(d);
    free(d);
}