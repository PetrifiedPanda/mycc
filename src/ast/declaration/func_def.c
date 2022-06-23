#include "ast/declaration/func_def.h"

void free_func_def_children(struct func_def* d) {
    free_declaration_specs(d->specs);
    free_declarator(d->decl);
    free_declaration_list(&d->decl_list);
    free_compound_statement(d->comp);
}

static bool visit_children(struct ast_visitor* visitor, struct func_def* d) {

    if (!visit_declaration_specs(visitor, d->specs)) {
        return false;
    }
    // TODO:
    /*
    if (!visit_declarator(visitor, d->decl)) {
        return false;
    }

    if (!visit_declaration_list(visitor, d->decl_list)) {
        return false;
    }

    if (!visit_compound_statement(visitor, d->comp)) {
        return false;
    }

    return true;
    */
    return false;
}

bool visit_func_def(struct ast_visitor* visitor, struct func_def* d) {
    AST_VISITOR_VISIT_TEMPLATE(visitor,
                               d,
                               visit_children,
                               visitor->visit_func_def);
}

