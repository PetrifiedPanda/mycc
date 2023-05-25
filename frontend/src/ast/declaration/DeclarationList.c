#include "frontend/ast/declaration/DeclarationList.h"

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

bool parse_declaration_list(ParserState* s, DeclarationList* res) {
    res->len = 1;
    res->decls = mycc_alloc(sizeof *res->decls);

    if (!parse_declaration_inplace(s, res->decls)) {
        mycc_free(res->decls);
        return false;
    }

    size_t alloc_size = res->len;
    while (is_declaration(s)) {
        if (res->len == alloc_size) {
            mycc_grow_alloc((void**)&res->decls, &alloc_size, sizeof *res->decls);
        }

        if (!parse_declaration_inplace(s, &res->decls[res->len])) {
            free_declaration_list(res);
            return false;
        }

        ++res->len;
    }

    return res;
}

void free_declaration_list(DeclarationList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_declaration_children(&l->decls[i]);
    }
    mycc_free(l->decls);
}

