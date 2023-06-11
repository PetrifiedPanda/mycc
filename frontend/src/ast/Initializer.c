#include "frontend/ast/Initializer.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

#include "frontend/ast/expr/AssignExpr.h"

#include "frontend/ast/Identifier.h"

static bool parse_designator_inplace(ParserState* s, Designator* res) {
    res->info = AstNodeInfo_create(s->it->loc);
    switch (s->it->kind) {
        case TOKEN_LINDEX: {
            parser_accept_it(s);
            ConstExpr* index = parse_const_expr(s);
            if (!index) {
                return false;
            }
            if (!parser_accept(s, TOKEN_RINDEX)) {
                ConstExpr_free(index);
                return false;
            }

            res->is_index = true;
            res->arr_index = index;
            return true;
        }
        case TOKEN_DOT: {
            parser_accept_it(s);
            if (s->it->kind == TOKEN_IDENTIFIER) {
                const StrBuf spell = Token_take_spelling(s->it);
                const SourceLoc loc = s->it->loc;
                parser_accept_it(s);
                res->is_index = false;
                res->identifier = Identifier_create(&spell, loc);
                return true;
            } else {
                expected_token_error(s, TOKEN_IDENTIFIER);
                return false;
            }
        }
        default: {
            static const TokenKind expected[] = {TOKEN_LINDEX, TOKEN_DOT};
            expected_tokens_error(s, expected, ARR_LEN(expected));
            return false;
        }
    }
}

void Designator_free_children(struct Designator* d) {
    if (d->is_index) {
        ConstExpr_free(d->arr_index);
    } else {
        Identifier_free(d->identifier);
    }
}

void DesignatorList_free(DesignatorList* l);

static bool parse_designator_list(ParserState* s, DesignatorList* res) {
    *res = (DesignatorList){
        .len = 1,
        .designators = mycc_alloc(sizeof *res->designators),
    };

    if (!parse_designator_inplace(s, &res->designators[0])) {
        mycc_free(res->designators);
        return false;
    }

    size_t alloc_len = res->len;
    while (s->it->kind == TOKEN_LINDEX || s->it->kind == TOKEN_DOT) {
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->designators,
                            &alloc_len,
                            sizeof *res->designators);
        }

        if (!parse_designator_inplace(s, &res->designators[res->len])) {
            goto fail;
        }

        ++res->len;
    }
    res->designators = mycc_realloc(res->designators,
                                    res->len * sizeof *res->designators);

    return res;

fail:
    DesignatorList_free(res);
    return false;
}

void DesignatorList_free(DesignatorList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        Designator_free_children(&l->designators[i]);
    }
    mycc_free(l->designators);
}

static bool parse_designation_inplace(ParserState* s, Designation* res) {
    if (!parse_designator_list(s, &res->designators)) {
        return false;
    }

    if (!parser_accept(s, TOKEN_ASSIGN)) {
        DesignatorList_free(&res->designators);
        return false;
    }

    return true;
}

void Designation_free_children(Designation* d) {
    DesignatorList_free(&d->designators);
}

void Designation_free(Designation* d) {
    Designation_free_children(d);
    mycc_free(d);
}

static bool parse_initializer_inplace(ParserState* s, Initializer* res);
bool Designation_is_valid(const Designation* d);

Designation create_invalid_designation(void) {
    return (Designation){
        .designators =
            {
                .len = 0,
                .designators = NULL,
            },
    };
}

static bool parse_designation_init(ParserState* s, DesignationInit* res) {
    if (s->it->kind == TOKEN_LINDEX || s->it->kind == TOKEN_DOT) {
        if (!parse_designation_inplace(s, &res->designation)) {
            return false;
        }
    } else {
        res->designation = create_invalid_designation();
    }

    if (!parse_initializer_inplace(s, &res->init)) {
        if (Designation_is_valid(&res->designation)) {
            Designation_free_children(&res->designation);
        }
        return false;
    }
    return true;
}

void InitList_free_children(InitList* l);

bool parse_init_list(ParserState* s, InitList* res) {
    *res = (InitList){
        .len = 1,
        .inits = mycc_alloc(sizeof *res->inits),
    };
    if (!parse_designation_init(s, &res->inits[0])) {
        mycc_free(res->inits);
        return false;
    }

    size_t alloc_len = res->len;
    while (s->it->kind == TOKEN_COMMA && s->it[1].kind != TOKEN_RBRACE) {
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->inits,
                            &alloc_len,
                            sizeof *res->inits);
        }

        if (!parse_designation_init(s, &res->inits[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->inits = mycc_realloc(res->inits, sizeof *res->inits * res->len);

    return res;
fail:
    InitList_free_children(res);
    return false;
}

bool Designation_is_valid(const Designation* d) {
    return d->designators.len != 0;
}

void InitList_free_children(InitList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        struct DesignationInit* item = &l->inits[i];
        if (Designation_is_valid(&item->designation)) {
            Designation_free_children(&item->designation);
        }
        Initializer_free_children(&item->init);
    }
    mycc_free(l->inits);
}

static bool parse_initializer_inplace(ParserState* s, Initializer* res) {
    res->info = AstNodeInfo_create(s->it->loc);
    if (s->it->kind == TOKEN_LBRACE) {
        res->is_assign = false;
        parser_accept_it(s);
        if (!parse_init_list(s, &res->init_list)) {
            return false;
        }

        if (s->it->kind == TOKEN_COMMA) {
            parser_accept_it(s);
        }

        if (!parser_accept(s, TOKEN_RBRACE)) {
            InitList_free_children(&res->init_list);
            return false;
        }
    } else {
        res->is_assign = true;
        res->assign = parse_assign_expr(s);
        if (!res->assign) {
            return false;
        }
    }
    return true;
}

Initializer* parse_initializer(ParserState* s) {
    struct Initializer* res = mycc_alloc(sizeof *res);
    if (!parse_initializer_inplace(s, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

void Initializer_free_children(Initializer* i) {
    if (i->is_assign) {
        AssignExpr_free(i->assign);
    } else {
        InitList_free_children(&i->init_list);
    }
}

void Initializer_free(struct Initializer* i) {
    Initializer_free_children(i);
    mycc_free(i);
}
