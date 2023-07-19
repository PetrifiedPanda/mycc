#include "frontend/ast/declaration/FuncDef.h"

#include "frontend/ast/declaration/DeclarationSpecs.h"
#include "frontend/ast/declaration/Declarator.h"

void FuncDef_free_children(FuncDef* d) {
    DeclarationSpecs_free(&d->specs);
    Declarator_free(d->decl);
    DeclarationList_free(&d->decl_list);
    CompoundStatement_free_children(&d->comp);
}

