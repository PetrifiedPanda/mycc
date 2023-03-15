#include "frontend/ast/type_name.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

bool parse_type_name_inplace(struct parser_state* s, struct type_name* res) {
    assert(res);

    if (is_type_spec(s) || is_type_qual(s->it->kind)) {
        res->spec_qual_list = mycc_alloc(sizeof *res->spec_qual_list);
        if (!parse_spec_qual_list(s, res->spec_qual_list)) {
            return false;
        }
    } else {
        // might be better for the error to just say "Expected type specifier or
        // type qualifier"
        enum token_kind expected[] = {
            VOID,     CHAR,     SHORT,    INT,  LONG,         FLOAT,
            DOUBLE,   SIGNED,   UNSIGNED, BOOL, COMPLEX,      IMAGINARY,
            ATOMIC,   STRUCT,   UNION,    ENUM, TYPEDEF_NAME, CONST,
            RESTRICT, VOLATILE, ATOMIC,
        };
        expected_tokens_error(s, expected, ARR_LEN(expected));
        return false;
    }

    if (s->it->kind == ASTERISK || s->it->kind == LBRACKET
        || s->it->kind == LINDEX) {
        res->abstract_decl = parse_abs_declarator(s);
        if (!res->abstract_decl) {
            free_spec_qual_list(res->spec_qual_list);
            return false;
        }
    } else {
        res->abstract_decl = NULL;
    }

    return true;
}

struct type_name* parse_type_name(struct parser_state* s) {
    struct type_name* res = mycc_alloc(sizeof *res);
    if (!parse_type_name_inplace(s, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

void free_type_name_children(struct type_name* n) {
    free_spec_qual_list(n->spec_qual_list);
    if (n->abstract_decl) {
        free_abs_declarator(n->abstract_decl);
    }
}

void free_type_name(struct type_name* n) {
    free_type_name_children(n);
    mycc_free(n);
}

