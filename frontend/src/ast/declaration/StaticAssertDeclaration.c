#include "frontend/ast/declaration/StaticAssertDeclaration.h"

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

#include "frontend/ast/AssignExpr.h"

StaticAssertDeclaration* parse_static_assert_declaration(ParserState* s) {
    if (!(ParserState_accept(s, TOKEN_STATIC_ASSERT)
          && ParserState_accept(s, TOKEN_LBRACKET))) {
        return NULL;
    }

    ConstExpr* const_expr = parse_const_expr(s);
    if (!const_expr) {
        return NULL;
    }

    if (!ParserState_accept(s, TOKEN_COMMA)) {
        ConstExpr_free(const_expr);
        return NULL;
    }

    if (ParserState_curr_kind(s) != TOKEN_STRING_LITERAL) {
        expected_token_error(s, TOKEN_STRING_LITERAL);
        ConstExpr_free(const_expr);
        return NULL;
    }

    const uint32_t idx = ParserState_curr_idx(s);
    ParserState_accept_it(s);

    if (!(ParserState_accept(s, TOKEN_RBRACKET)
          && ParserState_accept(s, TOKEN_SEMICOLON))) {
        ConstExpr_free(const_expr);
        return NULL;
    }

    StaticAssertDeclaration* res = mycc_alloc(sizeof *res);
    res->const_expr = const_expr;
    res->err_msg = StringLiteralNode_create(idx);

    return res;
}

static void StaticAssertDeclaration_free_children(StaticAssertDeclaration* d) {
    ConstExpr_free(d->const_expr);
}

void StaticAssertDeclaration_free(StaticAssertDeclaration* d) {
    StaticAssertDeclaration_free_children(d);

    mycc_free(d);
}

