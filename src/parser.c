#include "parser.h"

#include <stdbool.h>
#include <assert.h>

#include "error.h"
#include "util.h"

struct parser_state {
    struct token* it;
};

static struct translation_unit* parse_translation_unit(struct parser_state* s);

struct translation_unit* parse_tokens(struct token* tokens) {
    struct parser_state state = {tokens};
    struct translation_unit* res = parse_translation_unit(&state);
    assert(state.it->type == INVALID);
    return res;
}

static inline void expected_token_error(enum token_type expected, const struct token* got) {
    assert(got);

    set_error_file(ERR_PARSER, got->file, got->source_loc, "Expected token of type %s but got token of type %s", get_type_str(expected), get_type_str(got->type));
}

static inline void expected_tokens_error(const enum token_type* expected, size_t num_expected, const struct token* got) {
    assert(expected);
    assert(got);

    set_error_file(ERR_PARSER, got->file, got->source_loc, "Expected token of type %s", get_type_str(expected[0]));

    for (size_t i = 1; i < num_expected; ++i) {
        append_error_msg(", %s", get_type_str(expected[i]));
    }

    append_error_msg(" but got token of type %s", get_type_str(got->type));
}

static bool accept(struct parser_state* s, enum token_type expected) {
    if (s->it->type != expected) {
        expected_token_error(expected, s->it);
        return false;
    } else {
        ++s->it;
        return true;
    }
}

static void accept_it(struct parser_state* s) {
    ++s->it;
}

static bool parse_external_declaration(struct parser_state* s, struct external_declaration* res) {
    // TODO:
    return false;
}

static struct translation_unit* parse_translation_unit(struct parser_state* s) {
    struct translation_unit* res = xmalloc(sizeof(struct translation_unit));
    size_t alloc_num = 1;
    res->len = 0;
    res->external_decls = xmalloc(sizeof(struct external_declaration) * alloc_num);

    while (s->it->type != INVALID) {
        if (res->len == alloc_num) {
            grow_alloc((void**)res->external_decls, &alloc_num, sizeof(struct external_declaration));
        }
        
        if (!parse_external_declaration(s, &res->external_decls[res->len])) {
            goto fail;
        }

        ++res->len;
    }
    
    if (res->len != alloc_num) {
        res->external_decls = xrealloc(res->external_decls, res->len * sizeof(struct external_declaration));
    }

    return res;

fail:
    if (res) {
        free_translation_unit(res);
    }
    return NULL;
}

static bool parse_assign_expr(struct assign_expr* res, struct parser_state* s) {
    // TODO:
    return false;
}

static struct arg_expr_list parse_arg_expr_list(struct parser_state* s) {
    size_t alloc_size = 1;
    struct arg_expr_list res = {.len = 0, .assign_exprs = xmalloc(sizeof(struct assign_expr) * alloc_size)};
    if (!parse_assign_expr(&res.assign_exprs[0], s)) {
        goto fail;
    }
    res.len = 1;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (res.len == alloc_size) {
            grow_alloc((void**)&res.assign_exprs, &alloc_size, sizeof(struct assign_expr));
        }

        if (!parse_assign_expr(&res.assign_exprs[res.len], s)) {
            goto fail;
        }

        ++res.len;
    }

fail:
    free_arg_expr_list(&res);
    return (struct arg_expr_list){.assign_exprs = NULL, .len = 0};
}

static struct expr* parse_expr(struct parser_state* s);

static char* take_spelling(struct token* t) {
    char* spelling = t->spelling;
    t->spelling = NULL;
    return spelling;
}

static struct primary_expr* parse_primary_expr(struct parser_state* s) {
    switch (s->it->type) {
        case IDENTIFIER: {
            char* spelling = take_spelling(s->it);
            return create_primary_expr_identifier(create_identifier(spelling));
        }
        case I_CONSTANT: {
            char* spelling = take_spelling(s->it);
            return create_primary_expr_constant(create_constant(false, spelling));
        }
        case F_CONSTANT: {
            char* spelling = take_spelling(s->it);
            return create_primary_expr_constant(create_constant(true, spelling));
        }
        case STRING_LITERAL: {
            char* spelling = take_spelling(s->it);
            return create_primary_expr_string(create_string_literal(spelling));
        }
    
        default:
            if (accept(s, LBRACKET)) {
                struct expr* bracket_expr = parse_expr(s);
                if (!bracket_expr) {
                    return NULL;
                }
                if (accept(s, RBRACKET)) {
                    return create_primary_expr_bracket(bracket_expr);
                } else {
                    free_expr(bracket_expr);
                    expected_token_error(RBRACKET, s->it);
                    return NULL;
                }
            } else {
                enum token_type expected[] = {
                    IDENTIFIER,
                    I_CONSTANT,
                    F_CONSTANT,
                    STRING_LITERAL,
                    LBRACKET
                };
                size_t size = sizeof expected / sizeof(enum token_type);
                expected_tokens_error(expected, size, s->it);
                return NULL;
            }
    }

    return NULL;
}

static struct expr* parse_expr(struct parser_state* s) {
    size_t num_elems = 1;
    struct expr* res = xmalloc(sizeof(struct expr));
    res->assign_exprs = xmalloc(num_elems * sizeof(struct assign_expr));

    if (parse_assign_expr(&res->assign_exprs[0], s)) {
        res->len = 0;
        goto fail;
    }

    res->len = 1;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (num_elems == res->len) {
            grow_alloc((void**)&res->assign_exprs, &num_elems, sizeof(struct assign_expr));
        }

        if (!parse_assign_expr(&res->assign_exprs[res->len], s)) {
            goto fail;
        }

        ++res->len;
    }

    if (num_elems != res->len) {
        res->assign_exprs = xrealloc(res->assign_exprs, sizeof(struct assign_expr) * res->len);
    }

    return res;
fail:
    free_expr(res);
    return NULL;
}

static bool is_posfix_expr(enum token_type t) {
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

static struct postfix_expr* parse_postfix_expr(struct parser_state* s) {
    struct postfix_expr* res = xmalloc(sizeof(struct postfix_expr));
    res->primary = parse_primary_expr(s);
    if (!res->primary) {
        return NULL;
    }

    size_t alloc_size = 1;
    res->len = 0;
    res->suffixes = xmalloc(sizeof(struct postfix_suffix) * alloc_size);
    while (is_posfix_expr(s->it->type)) {
        if (res->len == alloc_size) {
            grow_alloc((void**)&res->suffixes, &alloc_size, sizeof(struct postfix_suffix));
        }

        switch (s->it->type) {
            case LINDEX: {
                accept_it(s);
                struct expr* expr = parse_expr(s);
                if (!expr) {
                    goto fail;
                }
                if (!accept(s, RINDEX)) {
                    free_expr(expr);
                    goto fail;
                }
                res->suffixes[res->len] = (struct postfix_suffix){
                    .type = POSTFIX_INDEX, 
                    .index_expr = expr};
                break;
            }
            
            case LBRACKET: {
                accept_it(s);
                struct arg_expr_list arg_expr_list = {.assign_exprs = NULL, .len = 0};
                if (s->it->type != RBRACKET) {
                    arg_expr_list = parse_arg_expr_list(s);
                    if (get_last_error() != ERR_NONE) {
                        goto fail;
                    }
                }
                accept(s, RBRACKET);
                res->suffixes[res->len] = (struct postfix_suffix){
                    .type = POSTFIX_BRACKET, 
                    .bracket_list = arg_expr_list
                    };
                break;
            }

            case DOT:
            case PTR_OP: {
                enum postfix_suffix_type type = s->it->type == PTR_OP 
                    	? POSTFIX_PTR_ACCESS : POSTFIX_ACCESS; 
                accept_it(s);
                if (s->it->type != IDENTIFIER) {
                    goto fail;                        
                }
                char* spelling = take_spelling(s->it);
                struct identifier* identifier = create_identifier(spelling);
                res->suffixes[res->len] = (struct postfix_suffix){
                    .type = type, 
                    .identifier = identifier};
                break;
            }

            case INC_OP:
            case DEC_OP: {
                enum token_type inc_dec = s->it->type;
                accept_it(s);
                res->suffixes[res->len] = (struct postfix_suffix){
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
        res->suffixes = xrealloc(res->suffixes, res->len * sizeof(struct postfix_suffix));
    }

    return res;
fail:
    free_postfix_expr(res);
    return NULL;
}
