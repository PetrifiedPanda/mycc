#include "ast/type_qual_list.h"

#include <stdlib.h>

static void free_children(TypeQualList* l) {
    
}

void free_type_qual_list(TypeQualList* l) {
    free_children(l);
    free(l);
}