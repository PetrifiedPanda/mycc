#include "frontend/ast/expr/UnaryExpr.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static bool parse_arg_expr_list(ParserState* s, ArgExprList* res) {
    res->len = 1;
    res->assign_exprs = mycc_alloc(sizeof *res->assign_exprs);
    if (!parse_assign_expr_inplace(s, &res->assign_exprs[0])) {
        mycc_free(res->assign_exprs);
        return false;
    }

    size_t alloc_len = res->len;
    while (s->it->kind == TOKEN_COMMA) {
        parser_accept_it(s);
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->assign_exprs,
                       &alloc_len,
                       sizeof *res->assign_exprs);
        }

        if (!parse_assign_expr_inplace(s, &res->assign_exprs[res->len])) {
            ArgExprList_free(res);
            return false;
        }

        ++res->len;
    }

    res->assign_exprs = mycc_realloc(res->assign_exprs,
                                 sizeof *res->assign_exprs * res->len);
    return res;
}

static Constant Constant_create(Value val, SourceLoc loc) {
    return (Constant){
        .info = AstNodeInfo_create(loc),
        .kind = CONSTANT_VAL,
        .val = val,
    };
}

static Constant Constant_create_enum(const Str* spelling,
                                     SourceLoc loc) {
    assert(spelling);
    return (Constant){
        .info = AstNodeInfo_create(loc),
        .kind = CONSTANT_ENUM,
        .spelling = *spelling,
    };
}

static StringConstant StringConstant_create(const StrLit* lit, SourceLoc loc) {
    assert(lit);
    return (StringConstant){
        .is_func = false,
        .lit = StringLiteralNode_create(lit, loc),
    };
}

static StringConstant StringConstant_create_func_name(SourceLoc loc) {
    return (StringConstant){
        .is_func = true,
        .info = AstNodeInfo_create(loc),
    };
}

static bool parse_primary_expr_inplace(ParserState* s, PrimaryExpr* res) {
    switch (s->it->kind) {
        case TOKEN_IDENTIFIER: {
            const Str spelling = Token_take_spelling(s->it);
            SourceLoc loc = s->it->loc;
            parser_accept_it(s);
            if (parser_is_enum_constant(s, &spelling)) {
                res->kind = PRIMARY_EXPR_CONSTANT;
                res->constant = Constant_create_enum(&spelling, loc);
                return true;
            }
            res->kind = PRIMARY_EXPR_IDENTIFIER;
            res->identifier = Identifier_create(&spelling, loc);
            return true;
        }
        case TOKEN_F_CONSTANT:
        case TOKEN_I_CONSTANT: {
            const SourceLoc loc = s->it->loc;
            const Value val = s->it->val;
            parser_accept_it(s);
            res->kind = PRIMARY_EXPR_CONSTANT;
            res->constant = Constant_create(val, loc);
            return true;
        }
        case TOKEN_STRING_LITERAL: {
            const StrLit lit = Token_take_str_lit(s->it);
            const SourceLoc loc = s->it->loc;
            parser_accept_it(s);
            res->kind = PRIMARY_EXPR_STRING_LITERAL;
            res->string = StringConstant_create(&lit, loc);
            return true;
        }
        case TOKEN_FUNC_NAME: {
            const SourceLoc loc = s->it->loc;
            parser_accept_it(s);
            res->kind = PRIMARY_EXPR_STRING_LITERAL;
            res->string = StringConstant_create_func_name(loc);
            return true;
        }
        case TOKEN_GENERIC: {
            res->kind = PRIMARY_EXPR_GENERIC;
            if (!parse_generic_sel_inplace(s, &res->generic)) {
                return false;
            }
            return true;
        }

        default: {
            const SourceLoc loc = s->it->loc;
            if (parser_accept(s, TOKEN_LBRACKET)) {
                Expr bracket_expr;
                if (!parse_expr_inplace(s, &bracket_expr)) {
                    return NULL;
                }
                if (parser_accept(s, TOKEN_RBRACKET)) {
                    res->kind = PRIMARY_EXPR_BRACKET;
                    res->bracket_expr = bracket_expr;
                    res->info = AstNodeInfo_create(loc);
                    return true;
                } else {
                    Expr_free_children(&bracket_expr);
                    return false;
                }
            }
        }
    }

    return false;
}

static bool is_posfix_op(TokenKind t) {
    switch (t) {
        case TOKEN_LINDEX:
        case TOKEN_LBRACKET:
        case TOKEN_DOT:
        case TOKEN_PTR_OP:
        case TOKEN_INC:
        case TOKEN_DEC:
            return true;

        default:
            return false;
    }
}

static bool parse_postfix_arr_suffix(ParserState* s, PostfixSuffix* res) {
    assert(res);
    assert(s->it->kind == TOKEN_LINDEX);

    parser_accept_it(s);
    Expr expr;
    if (!parse_expr_inplace(s, &expr)) {
        return false;
    }
    if (!parser_accept(s, TOKEN_RINDEX)) {
        Expr_free_children(&expr);
        return false;
    }

    res->kind = POSTFIX_INDEX;
    res->index_expr = expr;
    return true;
}

static bool parse_postfix_func_suffix(ParserState* s, PostfixSuffix* res) {
    assert(res);
    assert(s->it->kind == TOKEN_LBRACKET);

    parser_accept_it(s);
    ArgExprList arg_expr_list = {
        .assign_exprs = NULL,
        .len = 0,
    };
    if (s->it->kind != TOKEN_RBRACKET) {
        if (!parse_arg_expr_list(s, &arg_expr_list)) {
            return false;
        }
    }
    if (!parser_accept(s, TOKEN_RBRACKET)) {
        ArgExprList_free(&arg_expr_list);
        return false;
    }
    res->kind = POSTFIX_BRACKET;
    res->bracket_list = arg_expr_list;
    return true;
}

static bool parse_postfix_access_suffix(ParserState* s, PostfixSuffix* res) {
    assert(res);
    assert(s->it->kind == TOKEN_DOT || s->it->kind == TOKEN_PTR_OP);
    PostfixSuffixKind kind = s->it->kind == TOKEN_PTR_OP
                                        ? POSTFIX_PTR_ACCESS
                                        : POSTFIX_ACCESS;
    parser_accept_it(s);
    if (s->it->kind != TOKEN_IDENTIFIER) {
        return false;
    }
    const Str spelling = Token_take_spelling(s->it);
    const SourceLoc loc = s->it->loc;
    parser_accept_it(s);
    Identifier* identifier = Identifier_create(&spelling, loc);
    res->kind = kind;
    res->identifier = identifier;
    return true;
}

PostfixSuffix parse_postfix_inc_dec_suffix(ParserState* s) {
    assert(s->it->kind == TOKEN_INC || s->it->kind == TOKEN_DEC);
    const TokenKind op = s->it->kind;
    parser_accept_it(s);
    return (PostfixSuffix){
        .kind = op == TOKEN_INC ? POSTFIX_INC : POSTFIX_DEC,
    };
}

static bool parse_postfix_suffixes(ParserState* s, PostfixExpr* res) {
    size_t alloc_len = 0;
    while (is_posfix_op(s->it->kind)) {
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->suffixes,
                            &alloc_len,
                            sizeof *res->suffixes);
        }

        switch (s->it->kind) {
            case TOKEN_LINDEX:
                if (!parse_postfix_arr_suffix(s, &res->suffixes[res->len])) {
                    return false;
                }
                break;

            case TOKEN_LBRACKET:
                if (!parse_postfix_func_suffix(s, &res->suffixes[res->len])) {
                    return false;
                }
                break;

            case TOKEN_DOT:
            case TOKEN_PTR_OP:
                if (!parse_postfix_access_suffix(s, &res->suffixes[res->len])) {
                    return false;
                }
                break;

            case TOKEN_INC:
            case TOKEN_DEC:
                res->suffixes[res->len] = parse_postfix_inc_dec_suffix(s);
                break;

            default:
                UNREACHABLE();
        }

        ++res->len;
    }

    if (alloc_len != res->len) {
        res->suffixes = mycc_realloc(res->suffixes,
                                     sizeof *res->suffixes * res->len);
    }

    return true;
}

static bool parse_postfix_expr_inplace(ParserState* s, PostfixExpr* res) {
    res->suffixes = NULL;
    res->len = 0;

    if (s->it->kind == TOKEN_LBRACKET && next_is_type_name(s)) {
        res->info = AstNodeInfo_create(s->it->loc);
        parser_accept_it(s);

        res->is_primary = false;

        res->type_name = parse_type_name(s);
        if (!res->type_name) {
            return false;
        }

        if (!(parser_accept(s, TOKEN_RBRACKET)
              && parser_accept(s, TOKEN_LBRACE))) {
            TypeName_free(res->type_name);
            return false;
        }

        if (!parse_init_list(s, &res->init_list)) {
            TypeName_free(res->type_name);
            return false;
        }

        if (s->it->kind == TOKEN_COMMA) {
            parser_accept_it(s);
        }

        if (!parser_accept(s, TOKEN_RBRACE)) {
            PostfixExpr_free_children(res);
            return false;
        }
    } else {
        res->is_primary = true;
        if (!parse_primary_expr_inplace(s, &res->primary)) {
            return false;
        }
    }

    if (!parse_postfix_suffixes(s, res)) {
        PostfixExpr_free_children(res);
        return false;
    }

    return true;
}

/**
 *
 * @param s current state
 * @param type_name A type name that was already parsed by parse_unary_expr
 * @return A postfix_expr that uses the given type_name
 */
static bool parse_postfix_expr_type_name(ParserState* s, PostfixExpr* res, TypeName* type_name, SourceLoc start_bracket_loc) {
    assert(type_name);
    assert(s->it->kind == TOKEN_LBRACE);

    res->len = 0;
    res->suffixes = NULL;
    res->is_primary = false;
    res->info = AstNodeInfo_create(start_bracket_loc);
    res->type_name = type_name;

    res->init_list.len = 0;
    res->init_list.inits = NULL;

    parser_accept_it(s);

    if (!parse_init_list(s, &res->init_list)) {
        goto fail;
    }

    if (s->it->kind == TOKEN_COMMA) {
        parser_accept_it(s);
    }

    if (!parser_accept(s, TOKEN_RBRACE)) {
        return NULL;
    }

    if (!parse_postfix_suffixes(s, res)) {
        goto fail;
    }
    return res;
fail:
    PostfixExpr_free_children(res);
    return NULL;
}

static inline void assign_operators_before(UnaryExpr* res,
                                           UnaryExprOp* ops_before,
                                           size_t len) {
    assert(res);
    if (len > 0) {
        assert(ops_before);
    } else {
        assert(ops_before == NULL);
    }
    res->len = len;
    res->ops_before = ops_before;
}

static void UnaryExpr_init_postfix(UnaryExpr* res,
                                   UnaryExprOp* ops_before,
                                   size_t len,
                                   PostfixExpr postfix,
                                   SourceLoc loc) {
    res->info = AstNodeInfo_create(loc);
    assign_operators_before(res, ops_before, len);
    res->kind = UNARY_POSTFIX;
    res->postfix = postfix;
}

static bool is_unary_op(TokenKind k) {
    switch (k) {
        case TOKEN_AND:
        case TOKEN_ASTERISK:
        case TOKEN_ADD:
        case TOKEN_SUB:
        case TOKEN_BNOT:
        case TOKEN_NOT:
            return true;
        default:
            return false;
    }
}

static UnaryExprKind token_type_to_unary_expr_type(TokenKind t) {
    assert(is_unary_op(t));
    switch (t) {
        case TOKEN_AND:
            return UNARY_ADDRESSOF;
        case TOKEN_ASTERISK:
            return UNARY_DEREF;
        case TOKEN_ADD:
            return UNARY_PLUS;
        case TOKEN_SUB:
            return UNARY_MINUS;
        case TOKEN_BNOT:
            return UNARY_BNOT;
        case TOKEN_NOT:
            return UNARY_NOT;
        default:
            UNREACHABLE();
    }
}

static void UnaryExpr_init_unary_op(UnaryExpr* res,
                                    UnaryExprOp* ops_before,
                                    size_t len,
                                    TokenKind unary_op,
                                    CastExpr* cast_expr,
                                    SourceLoc loc) {
    assert(is_unary_op(unary_op));
    assert(cast_expr);
    res->info = AstNodeInfo_create(loc);
    assign_operators_before(res, ops_before, len);
    res->kind = token_type_to_unary_expr_type(unary_op);
    res->cast_expr = cast_expr;
}

static void UnaryExpr_init_sizeof_type(UnaryExpr* res,
                                       UnaryExprOp* ops_before,
                                       size_t len,
                                       TypeName* type_name,
                                       SourceLoc loc) {
    assert(type_name);
    res->info = AstNodeInfo_create(loc);
    assign_operators_before(res, ops_before, len);
    res->kind = UNARY_SIZEOF_TYPE;
    res->type_name = type_name;
}

static void UnaryExpr_init_alignof(UnaryExpr* res,
                                   UnaryExprOp* ops_before,
                                   size_t len,
                                   TypeName* type_name,
                                   SourceLoc loc) {
    assert(type_name);
    res->info = AstNodeInfo_create(loc);
    assign_operators_before(res, ops_before, len);
    res->kind = UNARY_ALIGNOF;
    res->type_name = type_name;
}

bool parse_unary_expr_type_name(ParserState* s,
                                UnaryExpr* res,
                                UnaryExprOp* ops_before,
                                size_t len,
                                TypeName* type_name,
                                SourceLoc start_bracket_loc) {
    assert(type_name);
    
    PostfixExpr postfix;
    if (!parse_postfix_expr_type_name(s, &postfix, type_name, start_bracket_loc)) {
        return false;
    }

    UnaryExpr_init_postfix(res, ops_before, len, postfix, start_bracket_loc);
    return true;
}

UnaryExprOp token_type_to_unary_expr_op(TokenKind t) {
    assert(t == TOKEN_INC || t == TOKEN_DEC || t == TOKEN_SIZEOF);
    switch (t) {
        case TOKEN_INC:
            return UNARY_OP_INC;
        case TOKEN_DEC:
            return UNARY_OP_DEC;
        case TOKEN_SIZEOF:
            return UNARY_OP_SIZEOF;

        default:
            UNREACHABLE();
    }
}

bool parse_unary_expr_inplace(ParserState* s, UnaryExpr* res) {
    size_t alloc_len = 0;
    UnaryExprOp* ops_before = NULL;

    const SourceLoc loc = s->it->loc;
    size_t len = 0;
    while (
        s->it->kind == TOKEN_INC || s->it->kind == TOKEN_DEC
        || (s->it->kind == TOKEN_SIZEOF && s->it[1].kind != TOKEN_LBRACKET)) {
        if (len == alloc_len) {
            mycc_grow_alloc((void**)&ops_before,
                            &alloc_len,
                            sizeof *ops_before);
        }

        ops_before[len] = token_type_to_unary_expr_op(s->it->kind);

        ++len;
        parser_accept_it(s);
    }

    if (ops_before) {
        ops_before = mycc_realloc(ops_before, len * sizeof *ops_before);
    }

    if (is_unary_op(s->it->kind)) {
        const TokenKind unary_op = s->it->kind;
        parser_accept_it(s);
        CastExpr* cast = parse_cast_expr(s);
        if (!cast) {
            goto fail;
        }
        UnaryExpr_init_unary_op(res, ops_before, len, unary_op, cast, loc);
        return true;
    } else {
        switch (s->it->kind) {
            case TOKEN_SIZEOF: {
                parser_accept_it(s);
                assert(s->it->kind == TOKEN_LBRACKET);
                const SourceLoc start_bracket_loc = s->it->loc;
                if (next_is_type_name(s)) {
                    parser_accept_it(s);

                    TypeName* type_name = parse_type_name(s);
                    if (!type_name) {
                        goto fail;
                    }

                    if (!parser_accept(s, TOKEN_RBRACKET)) {
                        goto fail;
                    }
                    if (s->it->kind == TOKEN_LBRACE) {
                        ++len;
                        ops_before = mycc_realloc(ops_before,
                                                  len * sizeof *ops_before);
                        ops_before[len - 1] = UNARY_OP_SIZEOF;

                        if (!parse_unary_expr_type_name(s,
                                                        res,
                                                        ops_before,
                                                        len,
                                                        type_name,
                                                        start_bracket_loc)) {
                            TypeName_free(type_name);
                            goto fail;
                        }
                        return true;
                    } else {
                        UnaryExpr_init_sizeof_type(res,
                                                   ops_before,
                                                   len,
                                                   type_name,
                                                   loc);
                        return true;
                    }
                } else {
                    ++len;
                    ops_before = mycc_realloc(ops_before,
                                              sizeof *ops_before * len);
                    ops_before[len - 1] = UNARY_OP_SIZEOF;

                    PostfixExpr postfix;
                    if (!parse_postfix_expr_inplace(s, &postfix)) {
                        goto fail;
                    }
                    UnaryExpr_init_postfix(res, ops_before, len, postfix, loc);
                    return true;
                }
            }
            case TOKEN_ALIGNOF: {
                parser_accept_it(s);
                if (!parser_accept(s, TOKEN_LBRACKET)) {
                    goto fail;
                }

                TypeName* type_name = parse_type_name(s);
                if (!type_name) {
                    goto fail;
                }

                if (!parser_accept(s, TOKEN_RBRACKET)) {
                    goto fail;
                }
                UnaryExpr_init_alignof(res, ops_before, len, type_name, loc);
                return true;
            }
            default: {
                PostfixExpr postfix;
                if (!parse_postfix_expr_inplace(s, &postfix)) {
                    goto fail;
                }
                UnaryExpr_init_postfix(res, ops_before, len, postfix, loc);
                return true;
            }
        }
    }
fail:
    mycc_free(ops_before);
    return NULL;
}

void Constant_free(Constant* c) {
    if (c->kind == CONSTANT_ENUM) {
        Str_free(&c->spelling);
    }
}

void StringConstant_free(StringConstant* c) {
    if (!c->is_func) {
        StringLiteralNode_free(&c->lit);
    }
}

void PrimaryExpr_free_children(PrimaryExpr* e) {
    switch (e->kind) {
        case PRIMARY_EXPR_IDENTIFIER:
            Identifier_free(e->identifier);
            break;

        case PRIMARY_EXPR_CONSTANT:
            Constant_free(&e->constant);
            break;

        case PRIMARY_EXPR_STRING_LITERAL:
            StringConstant_free(&e->string);
            break;

        case PRIMARY_EXPR_BRACKET:
            Expr_free_children(&e->bracket_expr);
            break;

        case PRIMARY_EXPR_GENERIC:
            GenericSel_free_children(&e->generic);
            break;
    }
}

void ArgExprList_free(ArgExprList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        AssignExpr_free_children(&l->assign_exprs[i]);
    }
    mycc_free(l->assign_exprs);
}

void PostfixExpr_free_children(PostfixExpr* p) {
    if (p->is_primary) {
        PrimaryExpr_free_children(&p->primary);
    } else {
        TypeName_free(p->type_name);
        InitList_free_children(&p->init_list);
    }
    for (size_t i = 0; i < p->len; ++i) {
        PostfixSuffix* s = &p->suffixes[i];
        switch (s->kind) {
            case POSTFIX_INDEX:
                Expr_free_children(&s->index_expr);
                break;
            case POSTFIX_BRACKET:
                ArgExprList_free(&s->bracket_list);
                break;
            case POSTFIX_ACCESS:
            case POSTFIX_PTR_ACCESS:
                Identifier_free(s->identifier);
                break;
            case POSTFIX_INC:
            case POSTFIX_DEC:
                break;
        }
    }
    mycc_free(p->suffixes);
}

void UnaryExpr_free_children(UnaryExpr* u) {
    mycc_free(u->ops_before);
    switch (u->kind) {
        case UNARY_POSTFIX:
            PostfixExpr_free_children(&u->postfix);
            break;
        case UNARY_ADDRESSOF:
        case UNARY_DEREF:
        case UNARY_PLUS:
        case UNARY_MINUS:
        case UNARY_BNOT:
        case UNARY_NOT:
            CastExpr_free(u->cast_expr);
            break;
        case UNARY_SIZEOF_TYPE:
        case UNARY_ALIGNOF:
            TypeName_free(u->type_name);
            break;
    }
}

