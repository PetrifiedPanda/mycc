#include "frontend/ast/Statement.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

#include "frontend/ast/Identifier.h"

static LabeledStatement* parse_labeled_statement(ParserState* s) {
    assert(ParserState_curr_kind(s) == TOKEN_CASE
           || ParserState_curr_kind(s) == TOKEN_IDENTIFIER
           || ParserState_curr_kind(s) == TOKEN_DEFAULT);
    LabeledStatement* res = mycc_alloc(sizeof *res);
    res->info = AstNodeInfo_create(ParserState_curr_loc(s));
    switch (ParserState_curr_kind(s)) {
        case TOKEN_CASE: {
            res->kind = LABELED_STATEMENT_CASE;
            ParserState_accept_it(s);
            ConstExpr case_expr;
            if (!parse_const_expr_inplace(s, &case_expr)) {
                mycc_free(res);
                return NULL;
            }
            res->case_expr = case_expr;
            break;
        }

        case TOKEN_IDENTIFIER: {
            res->kind = LABELED_STATEMENT_LABEL;
            const StrBuf spell = ParserState_take_curr_spell(s);
            const SourceLoc loc = ParserState_curr_loc(s);
            ParserState_accept_it(s);
            res->label = Identifier_create(&spell, loc);
            break;
        }

        case TOKEN_DEFAULT: {
            res->kind = LABELED_STATEMENT_DEFAULT;
            ParserState_accept_it(s);
            break;
        }
        default:
            UNREACHABLE();
    }

    if (!ParserState_accept(s, TOKEN_COLON)) {
        goto fail;
    }

    res->stat = parse_statement(s);
    if (!res->stat) {
        goto fail;
    }

    return res;
fail:
    if (res->kind == LABELED_STATEMENT_CASE) {
        ConstExpr_free_children(&res->case_expr);
    }
    mycc_free(res);
    return NULL;
}

static bool parse_block_item_inplace(ParserState* s, BlockItem* res) {
    if (is_declaration(s)) {
        res->is_decl = true;
        if (!parse_declaration_inplace(s, &res->decl)) {
            return false;
        }
    } else {
        res->is_decl = false;
        if (!parse_statement_inplace(s, &res->stat)) {
            return false;
        }
    }

    return true;
}

bool parse_compound_statement_inplace(ParserState* s, CompoundStatement* res) {
    res->info = AstNodeInfo_create(ParserState_curr_loc(s));
    if (!ParserState_accept(s, TOKEN_LBRACE)) {
        return false;
    }

    ParserState_push_scope(s);
    res->items = NULL;
    res->len = 0;

    size_t alloc_len = res->len;
    while (ParserState_curr_kind(s) != TOKEN_RBRACE) {
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->items,
                            &alloc_len,
                            sizeof *res->items);
        }

        if (!parse_block_item_inplace(s, &res->items[res->len])) {
            CompoundStatement_free_children(res);
            return false;
        }

        ++res->len;
    }

    res->items = mycc_realloc(res->items, sizeof *res->items * res->len);

    assert(ParserState_curr_kind(s) == TOKEN_RBRACE);
    ParserState_accept_it(s);

    ParserState_pop_scope(s);

    return true;
}

static struct CompoundStatement* parse_compound_statement(ParserState* s) {
    CompoundStatement* res = mycc_alloc(sizeof *res);
    if (!parse_compound_statement_inplace(s, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

static ExprStatement* parse_expr_statement(ParserState* s) {
    ExprStatement* res = mycc_alloc(sizeof *res);
    res->info = AstNodeInfo_create(ParserState_curr_loc(s));
    if (ParserState_curr_kind(s) == TOKEN_SEMICOLON) {
        ParserState_accept_it(s);
        res->expr.len = 0;
        res->expr.assign_exprs = NULL;
        return res;
    } else {
        if (!parse_expr_inplace(s, &res->expr)) {
            mycc_free(res);
            return NULL;
        }

        if (!ParserState_accept(s, TOKEN_SEMICOLON)) {
            ExprStatement_free(res);
            return NULL;
        }

        return res;
    }
}

static SelectionStatement* parse_selection_statement(ParserState* s) {
    assert(ParserState_curr_kind(s) == TOKEN_IF
           || ParserState_curr_kind(s) == TOKEN_SWITCH);
    SelectionStatement* res = mycc_alloc(sizeof *res);
    res->info = AstNodeInfo_create(ParserState_curr_loc(s));
    if (ParserState_curr_kind(s) == TOKEN_IF) {
        res->is_if = true;
    } else {
        res->is_if = false;
    }
    ParserState_accept_it(s);

    if (!ParserState_accept(s, TOKEN_LBRACKET)) {
        mycc_free(res);
        return NULL;
    }

    if (!parse_expr_inplace(s, &res->sel_expr)) {
        mycc_free(res);
        return NULL;
    }

    if (!ParserState_accept(s, TOKEN_RBRACKET)) {
        Expr_free_children(&res->sel_expr);
        mycc_free(res);
        return NULL;
    }

    res->sel_stat = parse_statement(s);
    if (!res->sel_stat) {
        Expr_free_children(&res->sel_expr);
        mycc_free(res);
        return NULL;
    }

    if (res->is_if && ParserState_curr_kind(s) == TOKEN_ELSE) {
        ParserState_accept_it(s);
        res->else_stat = parse_statement(s);
        if (!res->else_stat) {
            Statement_free(res->sel_stat);
            Expr_free_children(&res->sel_expr);
            mycc_free(res);
            return NULL;
        }
    } else {
        res->else_stat = NULL;
    }

    return res;
}

static void assign_do_or_while(SourceLoc loc,
                               IterationStatement* res,
                               Expr while_cond,
                               Statement* loop_body) {
    assert(res);
    assert(loop_body);
    res->info = AstNodeInfo_create(loc);
    res->while_cond = while_cond;
    res->loop_body = loop_body;
}

static IterationStatement* create_while_loop(SourceLoc loc,
                                             Expr while_cond,
                                             Statement* loop_body) {
    IterationStatement* res = mycc_alloc(sizeof *res);
    res->kind = ITERATION_STATEMENT_WHILE;
    assign_do_or_while(loc, res, while_cond, loop_body);

    return res;
}

static IterationStatement* create_do_loop(SourceLoc loc,
                                          Expr while_cond,
                                          Statement* loop_body) {
    IterationStatement* res = mycc_alloc(sizeof *res);
    res->kind = ITERATION_STATEMENT_DO;
    assign_do_or_while(loc, res, while_cond, loop_body);

    return res;
}

static IterationStatement* create_for_loop(SourceLoc loc,
                                           ForLoop for_loop,
                                           Statement* loop_body) {
    assert(for_loop.cond);
    assert(loop_body);
    struct IterationStatement* res = mycc_alloc(sizeof *res);
    res->info = AstNodeInfo_create(loc);
    res->kind = ITERATION_STATEMENT_FOR;
    res->loop_body = loop_body;
    res->for_loop = for_loop;

    return res;
}

static IterationStatement* parse_while_statement(ParserState* s,
                                                 SourceLoc loc) {
    assert(ParserState_curr_kind(s) == TOKEN_WHILE);

    ParserState_accept_it(s);

    if (!ParserState_accept(s, TOKEN_LBRACKET)) {
        return NULL;
    }

    Expr while_cond;
    if (!parse_expr_inplace(s, &while_cond)) {
        return NULL;
    }

    if (!ParserState_accept(s, TOKEN_RBRACKET)) {
        Expr_free_children(&while_cond);
        return NULL;
    }

    Statement* loop_body = parse_statement(s);
    if (!loop_body) {
        Expr_free_children(&while_cond);
        return NULL;
    }

    return create_while_loop(loc, while_cond, loop_body);
}

static IterationStatement* parse_do_loop(ParserState* s, SourceLoc loc) {
    assert(ParserState_curr_kind(s) == TOKEN_DO);

    ParserState_accept_it(s);

    Statement* loop_body = parse_statement(s);
    if (!loop_body) {
        return NULL;
    }

    if (!ParserState_accept(s, TOKEN_WHILE) || !ParserState_accept(s, TOKEN_LBRACKET)) {
        Statement_free(loop_body);
        return NULL;
    }

    Expr while_cond;
    if (!parse_expr_inplace(s, &while_cond)) {
        Statement_free(loop_body);
        return NULL;
    }
    if (!(ParserState_accept(s, TOKEN_RBRACKET)
          && ParserState_accept(s, TOKEN_SEMICOLON))) {
        Statement_free(loop_body);
        Expr_free_children(&while_cond);
        return NULL;
    }

    return create_do_loop(loc, while_cond, loop_body);
}

static IterationStatement* parse_for_loop(ParserState* s, SourceLoc loc) {
    assert(ParserState_curr_kind(s) == TOKEN_FOR);

    ParserState_accept_it(s);

    if (!ParserState_accept(s, TOKEN_LBRACKET)) {
        return NULL;
    }

    ForLoop loop;
    if (is_declaration(s)) {
        loop.is_decl = true;
        if (!parse_declaration_inplace(s, &loop.init_decl)) {
            return NULL;
        }
    } else {
        loop.is_decl = false;
        loop.init_expr = parse_expr_statement(s);
        if (!loop.init_expr) {
            return NULL;
        }
    }

    loop.cond = parse_expr_statement(s);
    if (!loop.cond) {
        if (loop.is_decl) {
            Declaration_free_children(&loop.init_decl);
        } else {
            ExprStatement_free(loop.init_expr);
        }
        return NULL;
    }

    if (ParserState_curr_kind(s) != TOKEN_RBRACKET) {
        if (!parse_expr_inplace(s, &loop.incr_expr)) {
            goto fail;
        }
    } else {
        loop.incr_expr = (Expr){
            .len = 0,
            .assign_exprs = NULL,
        };
    }

    if (!ParserState_accept(s, TOKEN_RBRACKET)) {
        goto fail;
    }

    Statement* loop_body = parse_statement(s);
    if (!loop_body) {
        goto fail;
    }

    return create_for_loop(loc, loop, loop_body);
fail:
    if (loop.is_decl) {
        Declaration_free_children(&loop.init_decl);
    } else {
        ExprStatement_free(loop.init_expr);
    }
    ExprStatement_free(loop.cond);
    return NULL;
}

static IterationStatement* parse_iteration_statement(ParserState* s) {
    assert(ParserState_curr_kind(s) == TOKEN_WHILE
           || ParserState_curr_kind(s) == TOKEN_DO
           || ParserState_curr_kind(s) == TOKEN_FOR);
    const SourceLoc loc = ParserState_curr_loc(s);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_WHILE:
            return parse_while_statement(s, loc);
        case TOKEN_DO:
            return parse_do_loop(s, loc);
        case TOKEN_FOR:
            return parse_for_loop(s, loc);

        default:
            UNREACHABLE();
    }
}

static JumpStatement* create_jump_statement(SourceLoc loc,
                                            JumpStatementKind kind) {
    JumpStatement* res = mycc_alloc(sizeof *res);
    res->info = AstNodeInfo_create(loc);
    res->kind = kind;

    return res;
}

static JumpStatement* create_goto_statement(SourceLoc loc,
                                            Identifier* identifier) {
    assert(identifier);
    JumpStatement* res = create_jump_statement(loc, JUMP_STATEMENT_GOTO);
    res->goto_label = identifier;

    return res;
}

static JumpStatement* create_return_statement(SourceLoc loc, Expr ret_val) {
    JumpStatement* res = create_jump_statement(loc, JUMP_STATEMENT_RETURN);
    res->ret_val = ret_val;
    return res;
}

void JumpStatement_free(JumpStatement* s);

static JumpStatement* parse_jump_statement(ParserState* s) {
    assert(ParserState_curr_kind(s) == TOKEN_GOTO
           || ParserState_curr_kind(s) == TOKEN_CONTINUE
           || ParserState_curr_kind(s) == TOKEN_BREAK
           || ParserState_curr_kind(s) == TOKEN_RETURN);
    const SourceLoc loc = ParserState_curr_loc(s);
    JumpStatement* res = NULL;
    switch (ParserState_curr_kind(s)) {
        case TOKEN_GOTO: {
            ParserState_accept_it(s);
            if (ParserState_curr_kind(s) == TOKEN_IDENTIFIER) {
                const StrBuf spell = ParserState_take_curr_spell(s);
                const SourceLoc id_loc = ParserState_curr_loc(s);
                ParserState_accept_it(s);
                res = create_goto_statement(loc,
                                            Identifier_create(&spell, id_loc));
                break;
            } else {
                expected_token_error(s, TOKEN_IDENTIFIER);
                return NULL;
            }
        }
        case TOKEN_CONTINUE:
        case TOKEN_BREAK: {
            const TokenKind t = ParserState_curr_kind(s);
            ParserState_accept_it(s);
            res = create_jump_statement(loc,
                                        t == TOKEN_CONTINUE
                                            ? JUMP_STATEMENT_CONTINUE
                                            : JUMP_STATEMENT_BREAK);
            res->ret_val = (Expr){
                .len = 0,
                .assign_exprs = NULL,
            };
            break;
        }
        case TOKEN_RETURN: {
            ParserState_accept_it(s);
            Expr ret_val;
            if (ParserState_curr_kind(s) == TOKEN_SEMICOLON) {
                ret_val = (Expr){
                    .len = 0,
                    .assign_exprs = NULL,
                };
            } else {
                if (!parse_expr_inplace(s, &ret_val)) {
                    return NULL;
                }
            }

            res = create_return_statement(loc, ret_val);
            break;
        }

        default:
            UNREACHABLE();
    }

    if (!ParserState_accept(s, TOKEN_SEMICOLON)) {
        JumpStatement_free(res);
        return NULL;
    }

    return res;
}

static void JumpStatement_free_children(JumpStatement* s) {
    switch (s->kind) {
        case JUMP_STATEMENT_GOTO:
            Identifier_free(s->goto_label);
            break;
        case JUMP_STATEMENT_CONTINUE:
        case JUMP_STATEMENT_BREAK:
            break;
        case JUMP_STATEMENT_RETURN:
            Expr_free_children(&s->ret_val);
            break;
    }
}

void JumpStatement_free(JumpStatement* s) {
    JumpStatement_free_children(s);
    mycc_free(s);
}

bool parse_statement_inplace(ParserState* s, Statement* res) {
    assert(res);

    switch (ParserState_curr_kind(s)) {
        case TOKEN_LBRACE: {
            res->kind = STATEMENT_COMPOUND;
            res->comp = parse_compound_statement(s);
            if (!res->comp) {
                return false;
            }
            break;
        }

        case TOKEN_FOR:
        case TOKEN_WHILE:
        case TOKEN_DO: {
            res->kind = STATEMENT_ITERATION;
            res->it = parse_iteration_statement(s);
            if (!res->it) {
                return false;
            }
            break;
        }

        case TOKEN_GOTO:
        case TOKEN_CONTINUE:
        case TOKEN_BREAK:
        case TOKEN_RETURN: {
            res->kind = STATEMENT_JUMP;
            res->jmp = parse_jump_statement(s);
            if (!res->jmp) {
                return false;
            }
            break;
        }

        case TOKEN_IF:
        case TOKEN_SWITCH: {
            res->kind = STATEMENT_SELECTION;
            res->sel = parse_selection_statement(s);
            if (!res->sel) {
                return false;
            }
            break;
        }

        case TOKEN_CASE:
        case TOKEN_DEFAULT: {
            res->kind = STATEMENT_LABELED;
            res->labeled = parse_labeled_statement(s);
            if (!res->labeled) {
                return false;
            }
            break;
        }

        case TOKEN_IDENTIFIER: {
            if (ParserState_next_token_kind(s) == TOKEN_COLON) {
                res->kind = STATEMENT_LABELED;
                res->labeled = parse_labeled_statement(s);
                if (!res->labeled) {
                    return false;
                }
                break;
            }
            goto is_stat_expr; // to avoid fallthrough warning
        }

is_stat_expr:
        default: {
            res->kind = STATEMENT_EXPRESSION;
            res->expr = parse_expr_statement(s);
            if (!res->expr) {
                return false;
            }
            break;
        }
    }
    return true;
}

Statement* parse_statement(ParserState* s) {
    Statement* res = mycc_alloc(sizeof *res);
    if (!parse_statement_inplace(s, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

static void LabeledStatement_free_children(LabeledStatement* s) {
    switch (s->kind) {
        case LABELED_STATEMENT_LABEL:
            Identifier_free(s->label);
            break;
        case LABELED_STATEMENT_CASE:
            ConstExpr_free_children(&s->case_expr);
            break;
        case LABELED_STATEMENT_DEFAULT:
            break;
        default:
            UNREACHABLE();
    }
    Statement_free(s->stat);
}

void LabeledStatement_free(LabeledStatement* s) {
    LabeledStatement_free_children(s);
    mycc_free(s);
}

static void BlockItem_free_children(BlockItem* i) {
    if (i->is_decl) {
        Declaration_free_children(&i->decl);
    } else {
        Statement_free_children(&i->stat);
    }
}

void CompoundStatement_free_children(CompoundStatement* s) {
    for (size_t i = 0; i < s->len; ++i) {
        BlockItem_free_children(&s->items[i]);
    }
    mycc_free(s->items);
}

void CompoundStatement_free(CompoundStatement* s) {
    CompoundStatement_free_children(s);
    mycc_free(s);
}

static void ExprStatement_free_children(ExprStatement* s) {
    Expr_free_children(&s->expr);
}

void ExprStatement_free(ExprStatement* s) {
    ExprStatement_free_children(s);
    mycc_free(s);
}

static void SelectionStatement_free_children(SelectionStatement* s) {
    Expr_free_children(&s->sel_expr);
    Statement_free(s->sel_stat);
    if (s->else_stat) {
        Statement_free(s->else_stat);
    }
}

void SelectionStatement_free(SelectionStatement* s) {
    SelectionStatement_free_children(s);
    mycc_free(s);
}

static void IterationStatement_free_children(IterationStatement* s) {
    switch (s->kind) {
        case ITERATION_STATEMENT_WHILE:
        case ITERATION_STATEMENT_DO:
            Expr_free_children(&s->while_cond);
            break;
        case ITERATION_STATEMENT_FOR: {
            if (s->for_loop.is_decl) {
                Declaration_free_children(&s->for_loop.init_decl);
            } else {
                ExprStatement_free(s->for_loop.init_expr);
            }
            ExprStatement_free(s->for_loop.cond);
            Expr_free_children(&s->for_loop.incr_expr);
            break;
        }
        default:
            UNREACHABLE();
    }
    Statement_free(s->loop_body);
}

void IterationStatement_free(IterationStatement* s) {
    IterationStatement_free_children(s);
    mycc_free(s);
}

void Statement_free_children(Statement* s) {
    switch (s->kind) {
        case STATEMENT_LABELED:
            LabeledStatement_free(s->labeled);
            break;
        case STATEMENT_COMPOUND:
            CompoundStatement_free(s->comp);
            break;
        case STATEMENT_EXPRESSION:
            ExprStatement_free(s->expr);
            break;
        case STATEMENT_SELECTION:
            SelectionStatement_free(s->sel);
            break;
        case STATEMENT_ITERATION:
            IterationStatement_free(s->it);
            break;
        case STATEMENT_JUMP:
            JumpStatement_free(s->jmp);
            break;
    }
}

void Statement_free(Statement* s) {
    Statement_free_children(s);
    mycc_free(s);
}
