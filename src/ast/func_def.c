#include "ast/func_def.h"

#include <stdlib.h>

void free_func_def_children(struct func_def* d) {
    if (d->specs) {
        free_declaration_specs(d->specs);
    }
    free_declarator(d->decl);
    free_declaration_list(&d->decl_list);
    free_compound_statement(d->comp);
}
