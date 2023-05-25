#include "frontend/ast/declaration/FuncDef.h"

void free_func_def_children(FuncDef* d) {
    free_declaration_specs(d->specs);
    free_declarator(d->decl);
    free_declaration_list(&d->decl_list);
    free_compound_statement_children(&d->comp);
}

