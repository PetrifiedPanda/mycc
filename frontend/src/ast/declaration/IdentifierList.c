#include "frontend/ast/declaration/IdentifierList.h"

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

#include "frontend/ast/Identifier.h"

bool parse_identifier_list(ParserState* s, IdentifierList* res) {
    if (ParserState_curr_kind(s) != TOKEN_IDENTIFIER) {
        return false;
    }
    *res = (IdentifierList){
        .len = 1,
        .identifiers = mycc_alloc(sizeof *res->identifiers),
    };
    uint32_t idx = s->it;
    ParserState_accept_it(s);
    Identifier_init(res->identifiers, idx);

    uint32_t alloc_len = res->len;
    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->identifiers,
                            &alloc_len,
                            sizeof *res->identifiers);
        }

        if (ParserState_curr_kind(s) != TOKEN_IDENTIFIER) {
            IdentifierList_free(res);
            return false;
        }
        idx = s->it;
        ParserState_accept_it(s);
        Identifier_init(&res->identifiers[res->len], idx);

        ++res->len;
    }

    res->identifiers = mycc_realloc(res->identifiers,
                                    sizeof *res->identifiers * res->len);

    return res;
}

void IdentifierList_free(IdentifierList* l) {
    mycc_free(l->identifiers);
}
