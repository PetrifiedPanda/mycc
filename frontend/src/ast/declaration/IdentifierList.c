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
    StrBuf spell = ParserState_take_curr_spell(s);
    SourceLoc loc = ParserState_curr_loc(s);
    ParserState_accept_it(s);
    Identifier_init(res->identifiers, &spell, loc);

    size_t alloc_len = res->len;
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
        spell = ParserState_take_curr_spell(s);
        loc = ParserState_curr_loc(s);
        ParserState_accept_it(s);
        Identifier_init(&res->identifiers[res->len], &spell, loc);

        ++res->len;
    }

    res->identifiers = mycc_realloc(res->identifiers,
                                    sizeof *res->identifiers * res->len);

    return res;
}

void IdentifierList_free(IdentifierList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        Identifier_free_children(&l->identifiers[i]);
    }
    mycc_free(l->identifiers);
}
