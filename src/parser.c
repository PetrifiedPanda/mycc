#include "parser.h"

#include <stdbool.h>
#include <assert.h>

#include "error.h"
#include "util.h"

struct parser_state {
    struct token* it;
};

static struct translation_unit parse_translation_unit(struct parser_state* s);

struct translation_unit parse_tokens(struct token* tokens) {
    struct parser_state state = {tokens};
    struct translation_unit res = parse_translation_unit(&state);
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
    assert(s->it->type != INVALID);
    ++s->it;
}

static bool parse_external_declaration(struct parser_state* s, struct external_declaration* res) {
    // TODO:
    return false;
}

static struct translation_unit parse_translation_unit(struct parser_state* s) {
    struct translation_unit res;
    size_t alloc_num = 1;
    res.len = 0;
    res.external_decls = xmalloc(sizeof(struct external_declaration) * alloc_num);

    while (s->it->type != INVALID) {
        if (res.len == alloc_num) {
            grow_alloc((void**)res.external_decls, &alloc_num, sizeof(struct external_declaration));
        }
        
        if (!parse_external_declaration(s, &res.external_decls[res.len])) {
            goto fail;
        }

        ++res.len;
    }
    
    if (res.len != alloc_num) {
        res.external_decls = xrealloc(res.external_decls, res.len * sizeof(struct external_declaration));
    }

    return res;

fail:
    free_translation_unit(&res);
    return (struct translation_unit){.len = 0, .external_decls = NULL};
}

static bool parse_assign_expr_inplace(struct parser_state* s, struct assign_expr* res) {
    // TODO:
    return false;
}

static struct assign_expr* parse_assign_expr(struct parser_state* s) {
    struct assign_expr* res = xmalloc(sizeof(struct assign_expr));
    if (!parse_assign_expr_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

static struct arg_expr_list parse_arg_expr_list(struct parser_state* s) {
    size_t alloc_size = 1;
    struct arg_expr_list res = {.len = 0, .assign_exprs = xmalloc(sizeof(struct assign_expr) * alloc_size)};
    if (!parse_assign_expr_inplace(s, &res.assign_exprs[0])) {
        goto fail;
    }
    res.len = 1;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (res.len == alloc_size) {
            grow_alloc((void**)&res.assign_exprs, &alloc_size, sizeof(struct assign_expr));
        }

        if (!parse_assign_expr_inplace(s, &res.assign_exprs[res.len])) {
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

static bool is_enum_constant(const char* spell) {
    // TODO:
    return false;
}

static bool parse_type_name_inplace(struct parser_state* s, struct type_name* res) {
    assert(res);
    // TODO:
    return NULL;
}

static struct type_name* parse_type_name(struct parser_state* s) {
    struct type_name* res = xmalloc(sizeof(struct type_name));
    if (!parse_type_name_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

static bool parse_generic_assoc_inplace(struct parser_state* s, struct generic_assoc* res) {
    if (s->it->type == DEFAULT) {
        res->type_name = NULL;
    } else {
        res->type_name = parse_type_name(s);
        if (!res->assign) {
            return false;
        }
    }

    if (!accept(s, COLON)) {
        goto fail;
    }

    res->assign = parse_assign_expr(s);
    if (!res->assign) {
        goto fail;
    }

    return true;
fail:
    if (res->type_name) {
        free_type_name(res->type_name);
    }
    return false;
}

static struct generic_assoc_list parse_generic_assoc_list(struct parser_state* s) {
    size_t alloc_size = 1;
    struct generic_assoc_list res = {
            .len = 1,
            .assocs = xmalloc(sizeof(struct generic_assoc) * alloc_size)};

    if (!parse_generic_assoc_inplace(s, &res.assocs[0])) {
        free(res.assocs);
        return (struct generic_assoc_list){.len = 0, .assocs = NULL};
    }

    while (s->it->type == COMMA) {
        accept_it(s);

        if (res.len == alloc_size) {
            grow_alloc((void**)&res.assocs, &alloc_size, sizeof(struct generic_assoc));
        }

        if (!parse_generic_assoc_inplace(s, &res.assocs[res.len])) {
            goto fail;
        }

        ++res.len;
    }
    res.assocs = xrealloc(res.assocs, res.len * sizeof(struct generic_assoc));

    return res;

fail:
    free_generic_assoc_list(&res);
    return (struct generic_assoc_list){.len = 0, .assocs = NULL};
}

struct generic_sel* parse_generic_sel(struct parser_state* s) {
    assert(s->it->type == GENERIC);
    accept_it(s);

    if (!accept(s, LBRACKET)) {
        return NULL;
    }

    struct assign_expr* assign = parse_assign_expr(s);
    if (!assign) {
        return NULL;
    }

    if (!accept(s, COMMA)) {
        goto fail;
    }

    struct generic_assoc_list assocs = parse_generic_assoc_list(s);
    if (assocs.len == 0) {
        goto fail;
    }

    return create_generic_sel(assign, assocs);

fail:
    free_assign_expr(assign);
    return NULL;
}

static struct primary_expr* parse_primary_expr(struct parser_state* s) {
    switch (s->it->type) {
        case IDENTIFIER: {
            char* spelling = take_spelling(s->it);
            accept_it(s);
            if (is_enum_constant(spelling)) {
                return create_primary_expr_constant(create_constant(ENUM, spelling));
            }
            return create_primary_expr_identifier(create_identifier(spelling));
        }
        case F_CONSTANT:
        case I_CONSTANT: {
            enum token_type type = s->it->type;
            char* spelling = take_spelling(s->it);
            accept_it(s);
            return create_primary_expr_constant(create_constant(type, spelling));
        }
        case STRING_LITERAL: {
            char* spelling = take_spelling(s->it);
            accept_it(s);
            return create_primary_expr_string(create_string_constant(spelling));
        }
        case FUNC_NAME: {
            accept_it(s);
            return create_primary_expr_string(create_func_name());
        }
        case GENERIC: {
            struct generic_sel* generic = parse_generic_sel(s);
            if (!generic) {
                return NULL;
            }
            return create_primary_expr_generic(generic);
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

    if (parse_assign_expr_inplace(s, &res->assign_exprs[0])) {
        res->len = 0;
        goto fail;
    }

    res->len = 1;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (num_elems == res->len) {
            grow_alloc((void**)&res->assign_exprs, &num_elems, sizeof(struct assign_expr));
        }

        if (!parse_assign_expr_inplace(&res->assign_exprs[res->len], s)) {
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

static bool is_posfix_op(enum token_type t) {
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

static bool is_type_qual(enum token_type t) {
    switch (t) {
        case CONST:
        case RESTRICT:
        case VOLATILE:
        case ATOMIC:
            return true;
        default:
            return false;
    }
}

static bool is_typedef_name(const struct parser_state* s, const char* spell) {
    // TODO:
    return false;
}

/**
 *
 * @param s current state
 * @return bool whether the next token could be the start of a type name
 * Disregards identifiers that may be type names
 */
static bool next_is_type_name(const struct parser_state* s) {
    assert(s->it->type != INVALID);
    struct token* next = s->it + 1;
    return is_keyword_type_spec(next->type) || is_type_qual(next->type) || (next->type == IDENTIFIER && is_typedef_name(s, next->spelling));
}

static struct init_list parse_init_list(struct parser_state* s) {
    // TODO:
    return (struct init_list){.len = 0, .inits = NULL};
}

static bool parse_postfix_suffixes(struct parser_state* s, struct postfix_expr* res) {
    size_t alloc_size = 0;
    while (is_posfix_op(s->it->type)) {
        if (res->len == alloc_size) {
            grow_alloc((void**)&res->suffixes, &alloc_size, sizeof(struct postfix_suffix));
        }

        switch (s->it->type) {
            case LINDEX: {
                accept_it(s);
                struct expr* expr = parse_expr(s);
                if (!expr) {
                    return false;
                }
                if (!accept(s, RINDEX)) {
                    free_expr(expr);
                    return false;
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
                        return false;
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
                    return false;
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
}

static struct postfix_expr* parse_postfix_expr(struct parser_state* s) {
    struct postfix_expr* res = xmalloc(sizeof(struct postfix_expr));
    res->suffixes = NULL;
    res->len = 0;

    if (s->it->type == LBRACKET && next_is_type_name(s)) {
        accept_it(s);

        res->is_primary = false;

        res->type_name = parse_type_name(s);
        if (!res->type_name) {
            free(res);
            return NULL;
        }

        if (!accept(s, LBRACE)) {
            goto fail;
        }

        res->init_list = parse_init_list(s);
        if (get_last_error() != ERR_NONE) {
            goto fail;
        }

        if (!accept(s, RBRACE)) {
            goto fail;
        }
    } else {
        res->is_primary = true;
        res->primary = parse_primary_expr(s);
        if (!res->primary) {
            free(res);
            return NULL;
        }
    }

    if (!parse_postfix_suffixes(s, res)) {
        goto fail;
    }

    return res;
fail:
    free_postfix_expr(res);
    return NULL;
}

/**
 *
 * @param s current state
 * @param type_name A type name that was already parsed by parse_unary_expr
 * @return A postfix_expr that uses the given type_name
 */
static struct postfix_expr* parse_postfix_expr_type_name(struct parser_state* s, struct type_name* type_name) {
    assert(s->it->type == LBRACE);

    struct postfix_expr* res = xmalloc(sizeof(struct postfix_expr));
    res->len = 0;
    res->suffixes = NULL;
    res->is_primary = false;
    res->type_name = type_name;

    res->init_list.len = 0;
    res->init_list.inits = NULL;

    accept_it(s);

    res->init_list = parse_init_list(s);
    if (get_last_error() != ERR_NONE) {
        goto fail;
    }

    if (!accept(s, RBRACE)) {
        return NULL;
    }

    if (!parse_postfix_suffixes(s, res)) {
        goto fail;
    }
    return res;
fail:
    free_postfix_expr(res);
    return NULL;
}

/**
 *
 * @param s current state
 * @param ops_before array of len tokens
 * @param len length of ops_before
 * @param type_name the already parsed type_name, with which this starts
 * @return struct unary_expr* unary expression created with the given parameters NULL on fail
 * This does not free any of the parameters
 */
static struct unary_expr* parse_unary_expr_type_name(struct parser_state* s, enum token_type* ops_before, size_t len, struct type_name* type_name) {
    struct postfix_expr* postfix = parse_postfix_expr_type_name(s, type_name);
    if (!postfix) {
        return NULL;
    }

    return create_unary_expr_postfix(ops_before, len, postfix);
}

static struct cast_expr* parse_cast_expr(struct parser_state* s);

static struct unary_expr* parse_unary_expr(struct parser_state* s) {
    size_t alloc_size = 0;
    enum token_type* ops_before = NULL;

    size_t len = 0;
    while (s->it->type == INC_OP || s->it->type == DEC_OP || (s->it->type == SIZEOF && (s->it + 1)->type != LBRACKET)) {
        if (len == alloc_size) {
            grow_alloc((void**)&ops_before, &alloc_size, sizeof(enum token_type));
        }

        ops_before[len] = s->it->type;

        ++len;
        accept_it(s);
    }
    ops_before = xrealloc(ops_before, len * sizeof(enum token_type));

    if (is_unary_op(s->it->type)) {
        enum token_type unary_op = s->it->type;
        accept_it(s);
        struct cast_expr* cast = parse_cast_expr(s);
        if (!cast) {
            goto fail;
        }
        return create_unary_expr_unary_op(ops_before, len, unary_op, cast);
    } else {
        switch (s->it->type) {
            case SIZEOF: {
                accept_it(s);
                assert(s->it->type == LBRACKET);
                if (next_is_type_name(s)) {
                    accept_it(s);

                    struct type_name* type_name = parse_type_name(s);
                    if (!type_name) {
                        goto fail;
                    }

                    if (!accept(s, RBRACKET)) {
                        goto fail;
                    }
                    if (s->it->type == LBRACE) {
                        ++len;
                        ops_before = xrealloc(ops_before, len * sizeof(enum token_type));
                        ops_before[len - 1] = SIZEOF;

                        struct unary_expr* res =  parse_unary_expr_type_name(s, ops_before, len, type_name);
                        if (!res) {
                            free_type_name(type_name);
                            goto fail;
                        }
                    } else {
                        return create_unary_expr_sizeof_type(ops_before, len, type_name);
                    }
                } else {
                    ++len;
                    ops_before = xrealloc(ops_before, len * sizeof(enum token_type));
                    ops_before[len - 1] = SIZEOF;

                    struct postfix_expr* postfix = parse_postfix_expr(s);
                    if (!postfix) {
                        goto fail;
                    }
                    return create_unary_expr_postfix(ops_before, len, postfix);
                }
                break; // unreachable
            }
            case ALIGNOF: {
                accept_it(s);
                if (!accept(s, LBRACKET)) {
                    goto fail;
                }

                struct type_name* type_name = parse_type_name(s);
                if (!type_name) {
                    goto fail;
                }

                if (!accept(s, RBRACKET)) {
                    goto fail;
                }
                return create_unary_expr_alignof(ops_before, len, type_name);
            }
            default: {
                struct postfix_expr* postfix = parse_postfix_expr(s);
                if (!postfix) {
                    goto fail;
                }
                return create_unary_expr_postfix(ops_before, len, postfix);
            }
        }
    }

    return NULL; // unreachable
fail:
    free(ops_before);
    return NULL;
}

static struct cast_expr* parse_cast_expr(struct parser_state* s) {
    struct type_name* type_names = NULL;
    size_t len = 0;

    size_t alloc_size = 0;
    while (s->it->type == LBRACKET && next_is_type_name(s)) {
        accept_it(s);

        if (len == alloc_size) {
            grow_alloc((void**)&type_names, &alloc_size, sizeof(struct type_name));
        }

        if (!parse_type_name_inplace(s, &type_names[len])) {
            goto fail;
        }

        if (!accept(s, LBRACKET)) {
            goto fail;
        }
        ++len;
    }

    struct unary_expr* rhs;
    if (s->it->type == LBRACE) {
        struct type_name* type_name = xmalloc(sizeof(struct type_name));

        --len;
        *type_name = type_names[len];

        type_names = xrealloc(type_names, len * sizeof(struct type_name));

        rhs = parse_unary_expr_type_name(s, NULL, 0, type_name);
    } else {
        type_names = xrealloc(type_names, len * sizeof(struct type_name));
        rhs = parse_unary_expr(s);
    }
    if (!rhs) {
        goto fail;
    }

    return create_cast_expr(type_names, len, rhs);

fail:
    for (size_t i = 0; i < len; ++i) {
        free_type_name_children(&type_names[i]);
    }
    free(type_names);
    return NULL;
}
