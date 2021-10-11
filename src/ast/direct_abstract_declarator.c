#include "ast/direct_abstract_declarator.h"

#include <stdlib.h>

static void free_children(DirectAbstractDeclarator* d) {

}

void free_direct_abstract_declarator(DirectAbstractDeclarator* d) {
    free_children(d);
    free(d);
}
