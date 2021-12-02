#include "parser.h"

#include <stdbool.h>
#include <assert.h>

#include "error.h"
#include "util.h"

typedef struct {
    Token* it;
} ParserState;

static TranslationUnit* parse_translation_unit(ParserState* s);

TranslationUnit* parse_tokens(Token* tokens) {
    ParserState state = {tokens};
    TranslationUnit* res = parse_translation_unit(&state);
    assert(state.it->type == INVALID);
    return res;
}

static void expected_token_error(TokenType expected, const Token* got) {
    set_error_file(ERR_PARSER, got->file, got->source_loc, "Expected token of type %s but got token of type %s", get_type_str(expected), get_type_str(got->type));
}

static bool accept(ParserState* s, TokenType expected) {
    if (s->it->type != expected) {
        expected_token_error(expected, s->it);
        return false;
    } else {
        ++s->it;
        return true;
    }
}

static void accept_it(ParserState* s) {
    ++s->it;
}

static bool parse_external_declaration(ParserState* s, ExternalDeclaration* res) {
    // TODO:
    return false;
}

static TranslationUnit* parse_translation_unit(ParserState* s) {
    TranslationUnit* res = xmalloc(sizeof(TranslationUnit));
    size_t alloc_num = 1;
    res->len = 0;
    res->external_decls = xmalloc(sizeof(ExternalDeclaration) * alloc_num);

    while (s->it->type != INVALID) {
        if (res->len == alloc_num) {
            grow_alloc((void**)res->external_decls, &alloc_num, sizeof(ExternalDeclaration));
        }
        
        if (!parse_external_declaration(s, &res->external_decls[res->len])) {
            goto fail;
        }

        ++res->len;
    }
    
    if (res->len != alloc_num) {
        res->external_decls = xrealloc(res->external_decls, res->len * sizeof(ExternalDeclaration));
    }

    return res;

fail:
    if (res) {
        free_translation_unit(res);
    }
    return NULL;
}

static bool parse_assign_expr(AssignExpr* res, ParserState* s) {
    // TODO:
    return false;
}

static ArgExprList parse_arg_expr_list(ParserState* s) {
    size_t alloc_size = 1;
    ArgExprList res = {.len = 0, .assign_exprs = xmalloc(sizeof(AssignExpr) * alloc_size)};
    if (!parse_assign_expr(&res.assign_exprs[0], s)) {
        goto fail;
    }
    res.len = 1;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (res.len == alloc_size) {
            grow_alloc((void**)&res.assign_exprs, &alloc_size, sizeof(AssignExpr));
        }

        if (!parse_assign_expr(&res.assign_exprs[res.len], s)) {
            goto fail;
        }

        ++res.len;
    }

fail:
    free_arg_expr_list(&res);
    return (ArgExprList){.assign_exprs = NULL, .len = 0};
}

static Expr* parse_expr(ParserState* s);

static char* take_spelling(Token* t) {
    char* spelling = t->spelling;
    t->spelling = NULL;
    return spelling;
}

static PrimaryExpr* parse_primary_expr(ParserState* s) {
    switch (s->it->type) {
        case IDENTIFIER: {
            char* spelling = take_spelling(s->it);
            return create_primary_expr_identifier(create_identifier(spelling));
        }
        case CONSTANT: {
            char* spelling = take_spelling(s->it);
            return create_primary_expr_constant(create_constant(spelling));
        }
        case STRING_LITERAL: {
            char* spelling = take_spelling(s->it);
            return create_primary_expr_string(create_string_literal(spelling));
        }
    
        default:
            if (accept(s, LBRACKET)) {
                Expr* bracket_expr = parse_expr(s);
                if (!bracket_expr) {
                    return NULL;
                }
                if (accept(s, RBRACKET)) {
                    return create_primary_expr_bracket(bracket_expr);
                } else {
                    free_expr(bracket_expr);
                    return NULL;
                }
            }
            break;
    }

    return NULL;
}

static Expr* parse_expr(ParserState* s) {
    size_t num_elems = 1;
    Expr* res = xmalloc(sizeof(Expr));
    res->assign_exprs = xmalloc(num_elems * sizeof(AssignExpr));

    if (parse_assign_expr(&res->assign_exprs[0], s)) {
        res->len = 0;
        goto fail;
    }

    res->len = 1;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (num_elems == res->len) {
            grow_alloc((void**)&res->assign_exprs, &num_elems, sizeof(AssignExpr));
        }

        if (!parse_assign_expr(&res->assign_exprs[res->len], s)) {
            goto fail;
        }

        ++res->len;
    }

    if (num_elems != res->len) {
        res->assign_exprs = xrealloc(res->assign_exprs, sizeof(AssignExpr) * res->len);
    }

    return res;
fail:
    free_expr(res);
    return NULL;
}

static bool is_posfix_expr(TokenType t) {
    switch (t) {
        case LINDEX:
        case LBRACKET:
        case DOT:
        case PTR_OP:
        case INC_OP:
        case DEC_OP:
            return true;
    
        default:
            return false;
    }
}

static PostfixExpr* parse_postfix_expr(ParserState* s) {
    PostfixExpr* res = xmalloc(sizeof(PostfixExpr));
    res->primary = parse_primary_expr(s);
    if (!res->primary) {
        return NULL;
    }

    size_t alloc_size = 1;
    res->len = 0;
    res->suffixes = xmalloc(sizeof(PostfixSuffix) * alloc_size);
    while (is_posfix_expr(s->it->type)) {
        if (res->len == alloc_size) {
            grow_alloc((void**)&res->suffixes, &alloc_size, sizeof(PostfixSuffix));
        }

        switch (s->it->type) {
            case LINDEX: {
                accept_it(s);
                Expr* expr = parse_expr(s);
                if (!expr) {
                    goto fail;
                }
                if (!accept(s, RINDEX)) {
                    free_expr(expr);
                    goto fail;
                }
                res->suffixes[res->len] = (PostfixSuffix){
                    .type = POSTFIX_INDEX, 
                    .index_expr = expr};
                break;
            }
            
            case LBRACKET: {
                accept_it(s);
                ArgExprList arg_expr_list = {.assign_exprs = NULL, .len = 0};
                if (s->it->type != RBRACKET) {
                    arg_expr_list = parse_arg_expr_list(s);
                    if (get_last_error() != ERR_NONE) {
                        goto fail;
                    }
                }
                accept(s, RBRACKET);
                res->suffixes[res->len] = (PostfixSuffix){
                    .type = POSTFIX_BRACKET, 
                    .bracket_list = arg_expr_list
                    };
                break;
            }

            case DOT:
            case PTR_OP: {
                PostfixSuffixType type = s->it->type == PTR_OP 
                    	? POSTFIX_PTR_ACCESS : POSTFIX_ACCESS; 
                accept_it(s);
                if (s->it->type != IDENTIFIER) {
                    goto fail;                        
                }
                char* spelling = take_spelling(s->it);
                Identifier* identifier = create_identifier(spelling); // Copy may not be necessary
                res->suffixes[res->len] = (PostfixSuffix){
                    .type = type, 
                    .identifier = identifier};
                break;
            }

            case INC_OP:
            case DEC_OP: {
                TokenType inc_dec = s->it->type;
                accept_it(s);
                res->suffixes[res->len] = (PostfixSuffix){
                    .type = POSTFIX_INC_DEC, 
                    .inc_dec = inc_dec};
                break;
            }

            default:
                assert(false); // Unreachable
        }

        ++res->len;
    }

    if (alloc_size != res->len) {
        res->suffixes = xrealloc(res->suffixes, res->len * sizeof(PostfixSuffix));
    }

    return res;
fail:
    free_postfix_expr(res);
    return NULL;
}
