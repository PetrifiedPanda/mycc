#include "frontend/ast/declaration/StaticAssertDeclaration.h"

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

StaticAssertDeclaration* parse_static_assert_declaration(ParserState* s) {
    if (!(parser_accept(s, TOKEN_STATIC_ASSERT)
          && parser_accept(s, TOKEN_LBRACKET))) {
        return NULL;
    }

    ConstExpr* const_expr = parse_const_expr(s);
    if (!const_expr) {
        return NULL;
    }

    if (!parser_accept(s, TOKEN_COMMA)) {
        ConstExpr_free(const_expr);
        return NULL;
    }

    if (s->it->kind != TOKEN_STRING_LITERAL) {
        expected_token_error(s, TOKEN_STRING_LITERAL);
        ConstExpr_free(const_expr);
        return NULL;
    }

    const StrLit lit = Token_take_str_lit(s->it);
    SourceLoc loc = s->it->loc;
    parser_accept_it(s);

    if (!(parser_accept(s, TOKEN_RBRACKET)
          && parser_accept(s, TOKEN_SEMICOLON))) {
        ConstExpr_free(const_expr);
        StrLit_free(&lit);
        return NULL;
    }

    StaticAssertDeclaration* res = mycc_alloc(sizeof *res);
    res->const_expr = const_expr;
    res->err_msg = StringLiteralNode_create(&lit, loc);

    return res;
}

static void free_static_assert_declaration_children(StaticAssertDeclaration* d) {
    ConstExpr_free(d->const_expr);
    StringLiteralNode_free(&d->err_msg);
}

void StaticAssertDeclaration_free(StaticAssertDeclaration* d) {
    free_static_assert_declaration_children(d);

    mycc_free(d);
}
