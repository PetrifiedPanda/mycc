#include "frontend/ast/ast.h"

#include <string.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static void AST_ensure_capacity(AST* ast) {
    if (ast->len == ast->cap) {
        mycc_grow_alloc((void**)&ast->kinds, &ast->cap, sizeof *ast->kinds);
        ast->datas = mycc_realloc(ast->datas, sizeof *ast->datas * ast->cap);
    }
}

static uint32_t add_node(AST* ast,
                         ASTNodeKind kind,
                         uint32_t main_token,
                         uint32_t rhs,
                         bool alloc_type_data) {
    AST_ensure_capacity(ast);

    uint32_t type_data_idx = (uint32_t)-1;
    if (alloc_type_data) {
        if (ast->type_data_len == ast->type_data_cap) {
            mycc_grow_alloc((void**)&ast->type_data,
                            &ast->type_data_cap,
                            sizeof *ast->type_data);
        }
        type_data_idx = ast->type_data_len;
        ++ast->type_data_len;
    }
    const uint32_t idx = ast->len;
    ast->kinds[ast->len] = kind;
    ast->datas[ast->len] = (ASTNodeData){
        .main_token = main_token,
        .rhs = rhs,
        .type_data_idx = type_data_idx,
    };

    ++ast->len;
    return idx;
}

static uint32_t add_node_without_children(AST* ast,
                                          ASTNodeKind kind,
                                          uint32_t main_token) {
    return add_node(ast, kind, main_token, 0, true);
}

static uint32_t parse_expr_2(ParserState* s, AST* ast) {
    (void)s;
    (void)ast;
    // TODO:
    return 0;
}

static uint32_t parse_assign_expr_2(ParserState* s, AST* ast) {
    (void)s;
    (void)ast;
    // TODO:
    return 0;
}

static uint32_t parse_type_name_2(ParserState* s, AST* ast) {
    (void)s;
    (void)ast;
    // TODO:
    return 0;
}

static uint32_t parse_generic_assoc_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_GENERIC_ASSOC, s->_it, 0, true);
    if (ParserState_curr_kind(s) == TOKEN_DEFAULT) {
        ParserState_accept_it(s);
    } else {
        if (parse_type_name_2(s, ast) == 0) {
            return 0;
        }
    }

    if (!ParserState_accept(s, TOKEN_COLON)) {
        return 0;
    }

    const uint32_t rhs = parse_assign_expr_2(s, ast);
    if (rhs == 0) {
        return 0;
    }
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_generic_assoc_list_2(ParserState* s, AST* ast) {
    // TODO: this probably does not need a node
    const uint32_t res = add_node(ast, AST_GENERIC_ASSOC_LIST, s->_it, 0, true);

    if (parse_generic_assoc_2(s, ast) == 0) {
        return 0;
    }

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);

        if (parse_generic_assoc_2(s, ast) == 0) {
            return 0;
        }
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_generic_sel_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_GENERIC);
    const uint32_t res = add_node(ast, AST_GENERIC_SEL, s->_it, 0, true);
    ParserState_accept_it(s);
    if (!ParserState_accept(s, TOKEN_LBRACKET)) {
        return 0;
    }
    // lhs
    if (parse_assign_expr_2(s, ast) == 0) {
        return 0;
    }

    if (!ParserState_accept(s, TOKEN_COMMA)) {
        return 0;
    }

    const uint32_t rhs = parse_generic_assoc_list_2(s, ast);
    if (rhs == 0) {
        return 0;
    }
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_primary_expr_2(ParserState* s, AST* ast) {
    // TODO: this might not need a node
    const uint32_t res = add_node(ast, AST_PRIMARY_EXPR, s->_it, 0, true);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_IDENTIFIER:
            if (ParserState_is_enum_constant(s, ParserState_curr_spell(s))) {
                add_node_without_children(ast, AST_ENUM_CONSTANT, s->_it);
            } else {
                add_node_without_children(ast, AST_IDENTIFIER, s->_it);
            }
            ParserState_accept_it(s);
            break;
        case TOKEN_I_CONSTANT:
        case TOKEN_F_CONSTANT:
            add_node_without_children(ast, AST_CONSTANT, s->_it);
            ParserState_accept_it(s);
            break;
        case TOKEN_STRING_LITERAL:
            add_node_without_children(ast, AST_STRING_LITERAL, s->_it);
            ParserState_accept_it(s);
            break;
        case TOKEN_FUNC_NAME:
            add_node_without_children(ast, AST_FUNC, s->_it);
            ParserState_accept_it(s);
            break;
        case TOKEN_LBRACKET:
            ParserState_accept_it(s);
            if (parse_expr_2(s, ast) == 0) {
                return 0;
            }
            if (!ParserState_accept(s, TOKEN_RBRACKET)) {
                return 0;
            }
            break;
        case TOKEN_GENERIC:
            parse_generic_sel_2(s, ast);
            break;
        default: {
            static const TokenKind ex[] = {
                TOKEN_IDENTIFIER,
                TOKEN_I_CONSTANT,
                TOKEN_F_CONSTANT,
                TOKEN_STRING_LITERAL,
                TOKEN_FUNC_NAME,
                TOKEN_LBRACKET,
            };
            expected_tokens_error(s, ex, ARR_LEN(ex));
            return 0;
        }
    }

    return res;
}

static uint32_t parse_storage_class_specs_2(ParserState* s, AST* ast) {
    assert(is_storage_class_spec(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast,
                                  AST_STORAGE_CLASS_SPECS,
                                  s->_it,
                                  0,
                                  false);
    ParserState_accept_it(s);
    uint32_t len = 1;
    while (is_storage_class_spec(ParserState_curr_kind(s))) {
        ++len;
        ParserState_accept_it(s);
    }
    // here rhs is not a node index, but how many tokens from token_idx are
    // storage_class specs
    ast->datas[res].rhs = len;

    return res;
}

static uint32_t parse_compound_literal_type_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast,
                                  AST_COMPOUND_LITERAL_TYPE,
                                  s->_it,
                                  0,
                                  false);
    if (is_storage_class_spec(ParserState_curr_kind(s))) {
        if (parse_storage_class_specs_2(s, ast) == 0) {
            return 0;
        }
    }

    const uint32_t rhs = parse_type_name_2(s, ast);
    ast->datas[res].rhs = rhs;
    return res;
}

static bool is_designator(TokenKind k) {
    switch (k) {
        case TOKEN_LINDEX:
        case TOKEN_DOT:
            return true;
        default:
            return false;
    }
}

static uint32_t parse_const_expr_2(ParserState* s, AST* ast) {
    (void)s;
    (void)ast;
    // TODO:
    return 0;
}

static uint32_t parse_designator_2(ParserState* s, AST* ast) {
    assert(is_designator(ParserState_curr_kind(s)));
    // TODO: might not need node
    const uint32_t res = add_node(ast, AST_DESIGNATOR, s->_it, 0, true);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_LINDEX:
            ParserState_accept_it(s);
            if (parse_const_expr_2(s, ast) == 0) {
                return 0;
            }
            if (!ParserState_accept(s, TOKEN_RINDEX)) {
                return 0;
            }
            break;
        case TOKEN_DOT:
            ParserState_accept_it(s);
            const uint32_t id_idx = s->_it;
            if (!ParserState_accept(s, TOKEN_IDENTIFIER)) {
                return 0;
            }
            add_node_without_children(ast, AST_IDENTIFIER, id_idx);
            break;
        default:
            UNREACHABLE();
    }
    return res;
}

static uint32_t parse_designator_list_2(ParserState* s, AST* ast) {
    assert(is_designator(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast, AST_DESIGNATOR_LIST, s->_it, 0, false);
    if (parse_designator_2(s, ast) == 0) {
        return 0;
    }

    while (is_designator(ParserState_curr_kind(s))) {
        if (parse_designator_2(s, ast) == 0) {
            return 0;
        }
    }

    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_braced_initializer_2(ParserState* s, AST* ast);

static uint32_t parse_initializer_2(ParserState* s, AST* ast) {
    // TODO: Might not need a node
    const uint32_t res = add_node(ast, AST_INITIALIZER, s->_it, 0, false);
    if (ParserState_curr_kind(s) == TOKEN_RBRACE) {
        if (parse_braced_initializer_2(s, ast) == 0) {
            return 0;
        }
    } else {
        if (parse_assign_expr_2(s, ast) == 0) {
            return 0;
        }
    }
    return res;
}

static uint32_t parse_designation_init_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_DESIGNATION_INIT, s->_it, 0, false);
    if (is_designator(ParserState_curr_kind(s))) {
        if (parse_designator_list_2(s, ast) == 0) {
            return 0;
        }

        if (!ParserState_accept(s, TOKEN_ASSIGN)) {
            return 0;
        }
    }

    const uint32_t rhs = parse_initializer_2(s, ast);
    if (rhs == 0) {
        return 0;
    }
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_init_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_INIT_LIST, s->_it, 0, false);
    if (parse_designation_init_2(s, ast) == 0) {
        return 0;
    }

    while (ParserState_curr_kind(s) == TOKEN_COMMA && ParserState_next_token_kind(s) != TOKEN_RBRACE) {
        ParserState_accept_it(s);

        if (parse_designation_init_2(s, ast) == 0) {
            return 0;
        }
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_braced_initializer_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_BRACED_INITIALIZER, s->_it, 0, false);
    if (!ParserState_accept(s, TOKEN_LBRACE)) {
        return 0;
    }

    if (ParserState_curr_kind(s) != TOKEN_RBRACE) {
        const uint32_t rhs = parse_init_list_2(s, ast);
        if (rhs == 0) {
            return 0;
        }
        
        if (ParserState_curr_kind(s) == TOKEN_COMMA) {
            ParserState_accept_it(s);
        }
        if (!ParserState_accept(s, TOKEN_RBRACE)) {
            return 0;
        }
        ast->datas[res].rhs = rhs;
        return res;
    } else {
        ParserState_accept_it(s);
        return res;
    }
}

static uint32_t parse_compound_literal_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET);
    const uint32_t res = add_node(ast, AST_COMPOUND_LITERAL, s->_it, 0, true);
    ParserState_accept_it(s);
    if (parse_compound_literal_type_2(s, ast) == 0) {
        return 0;
    }
    if (!ParserState_accept(s, TOKEN_RBRACKET)) {
        return 0;
    }
    
    const uint32_t rhs = parse_braced_initializer_2(s, ast);
    if (rhs == 0) {
        return 0;
    }
    ast->datas[res].rhs = rhs;
    return res;
}

static bool is_postfix_op(TokenKind t) {
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

static uint32_t parse_arg_expr_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_ARG_EXPR_LIST, s->_it, 0, false);
    // list is empty
    if (ParserState_curr_kind(s) == TOKEN_RBRACKET) {
        ast->datas[res].rhs = ast->len;
        return res;
    }

    if (parse_assign_expr_2(s, ast) == 0) {
        return 0;
    }

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);

        if (parse_assign_expr_2(s, ast) == 0) {
            return 0;
        }
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

uint32_t parse_postfix_expr_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_POSTFIX_EXPR, s->_it, 0, true);

    if (ParserState_curr_kind(s) == TOKEN_LBRACKET && next_is_type_name(s)) {
        parse_compound_literal_2(s, ast);
    } else {
        parse_primary_expr_2(s, ast);
    }

    uint32_t curr_node_idx = res;
    while (is_postfix_op(ParserState_curr_kind(s))) {
        const uint32_t rhs = add_node(ast,
                                      AST_TRANSLATION_UNIT,
                                      s->_it,
                                      0,
                                      true);
        switch (ParserState_curr_kind(s)) {
            case TOKEN_LINDEX:
                ast->kinds[rhs] = AST_POSTFIX_OP_INDEX;
                ParserState_accept_it(s);
                if (parse_expr_2(s, ast) == 0) {
                    return 0;
                }
                if (!ParserState_accept(s, TOKEN_RINDEX)) {
                    return 0;
                }
                break;
            case TOKEN_LBRACKET:
                ast->kinds[rhs] = AST_POSTFIX_OP_CALL;
                ParserState_accept_it(s);
                if (parse_arg_expr_list_2(s, ast) == 0) {
                    return 0;
                }
                if (!ParserState_accept(s, TOKEN_RBRACKET)) {
                    return 0;
                }
                break;
            case TOKEN_DOT:
            case TOKEN_PTR_OP:
                ast->kinds[rhs] = AST_POSTFIX_OP_ACCESS;
                ParserState_accept_it(s);
                const uint32_t id_idx = s->_it;
                if (!ParserState_accept(s, TOKEN_IDENTIFIER)) {
                    return 0;
                }
                add_node_without_children(ast, AST_IDENTIFIER, id_idx);
                break;
            case TOKEN_INC:
            case TOKEN_DEC:
                ast->kinds[rhs] = AST_POSTFIX_OP_INC_DEC;
                ParserState_accept_it(s);
                break;
            default:
                UNREACHABLE();
        }
        ast->datas[curr_node_idx].rhs = rhs;
        assert(ast->kinds[rhs] != AST_TRANSLATION_UNIT);
        curr_node_idx = rhs;
    }

    return res;
}

