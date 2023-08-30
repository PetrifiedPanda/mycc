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
                         bool alloc_type_data) {
    AST_ensure_capacity(ast);

    uint32_t type_data_idx = (uint32_t)-1;
    if (alloc_type_data) {
        type_data_idx = ast->type_data_len;
        ++ast->type_data_len;
    }
    const uint32_t idx = ast->len;
    ast->kinds[ast->len] = kind;
    ast->datas[ast->len] = (ASTNodeData){
        .main_token = main_token,
        .rhs = 0, // has to be initialized after (because we don't have rhs yet)
        .type_data_idx = type_data_idx,
    };

    ++ast->len;
    return idx;
}

#define CHECK_ERR(expr)                                                        \
    do {                                                                       \
        if (!(expr)) {                                                         \
            return 0;                                                          \
        }                                                                      \
    } while (0)

static uint32_t parse_type_name_2(ParserState* s, AST* ast);

static uint32_t parse_atomic_type_spec_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_ATOMIC);
    const uint32_t res = add_node(ast, AST_ATOMIC_TYPE_SPEC, s->it, false);
    ParserState_accept_it(s);
    CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));
    CHECK_ERR(parse_type_name_2(s, ast));
    CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
    return res;
}

static uint32_t parse_const_expr_2(ParserState* s, AST* ast);

static uint32_t parse_static_assert_declaration_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_STATIC_ASSERT);
    const uint32_t res = add_node(ast,
                                  AST_STATIC_ASSERT_DECLARATION,
                                  s->it,
                                  false);
    ParserState_accept_it(s);
    CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));

    CHECK_ERR(parse_const_expr_2(s, ast));

    // TODO: message not optional in earlier c versions
    if (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);
        const uint32_t lit_idx = s->it;
        CHECK_ERR(ParserState_accept(s, TOKEN_STRING_LITERAL));
        // TODO: type might not be necessary in this node
        ast->datas[res].rhs = add_node(ast, AST_STRING_LITERAL, lit_idx, true);
    }

    CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET)
              && ParserState_accept(s, TOKEN_SEMICOLON));
    return res;
}

static uint32_t parse_attribute_spec_sequence_2(ParserState* s, AST* ast);

static uint32_t parse_id_attribute_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_IDENTIFIER);
    const uint32_t res = add_node(ast, AST_ID_ATTRIBUTE, s->it, true);
    add_node(ast, AST_IDENTIFIER, s->it, true);
    ParserState_accept_it(s);

    if (ParserState_curr_kind(s) == TOKEN_LINDEX
        && ParserState_next_token_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_attribute_spec_sequence_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_type_qual_list_2(ParserState* s, AST* ast);
static uint32_t parse_assign_expr_2(ParserState* s, AST* ast);

static uint32_t parse_arr_suffix_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t idx = s->it;
    ParserState_accept_it(s);
    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_STATIC) {
        ParserState_accept_it(s);
        const uint32_t res = add_node(ast, AST_ARR_SUFFIX_STATIC, idx, false);
        if (is_type_qual(ParserState_curr_kind(s))) {
            CHECK_ERR(parse_type_qual_list_2(s, ast));
        }

        const uint32_t rhs = parse_assign_expr_2(s, ast);
        CHECK_ERR(rhs);

        CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
        ast->datas[res].rhs = rhs;
        return res;
    } else if (curr_kind == TOKEN_ASTERISK) {
        ParserState_accept_it(s);
        CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
        return add_node(ast, AST_ARR_SUFFIX_ASTERISK, idx, false);
    } else if (is_type_qual(curr_kind)) {
        const uint32_t res = add_node(ast, AST_ARR_SUFFIX, idx, false);
        CHECK_ERR(parse_type_qual_list_2(s, ast));

        switch (ParserState_curr_kind(s)) { 
            case TOKEN_ASTERISK: {
                ast->kinds[res] = AST_ARR_SUFFIX_ASTERISK;
                ParserState_accept_it(s);
                CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
                return res;
            }
            case TOKEN_RINDEX: {
                ParserState_accept_it(s);
                return res;
            }
            case TOKEN_STATIC:
                ast->kinds[res] = AST_ARR_SUFFIX_STATIC;
                ParserState_accept_it(s);
                FALLTHROUGH();
            default: {
                const uint32_t rhs = parse_assign_expr_2(s, ast);
                CHECK_ERR(rhs);
                CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
                ast->datas[res].rhs = rhs;
                return res;
            }
        }

    } else if (curr_kind == TOKEN_RINDEX) {
        ParserState_accept_it(s);
        return add_node(ast, AST_ARR_SUFFIX, idx, false);
    } else {
        const uint32_t res = add_node(ast, AST_ARR_SUFFIX, idx, false);
        const uint32_t rhs = parse_assign_expr_2(s, ast);
        CHECK_ERR(rhs);
        CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
        ast->datas[res].rhs = rhs;
        return res;
    }
}

static uint32_t parse_identifier_list_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_IDENTIFIER);
    const uint32_t res = add_node(ast, AST_IDENTIFIER_LIST, s->it, false);

    // TODO: does this need a type?
    add_node(ast, AST_IDENTIFIER, s->it, true);
    ParserState_accept_it(s);

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);
        const uint32_t curr = s->it;
        CHECK_ERR(ParserState_accept(s, TOKEN_IDENTIFIER));
        // TODO: does this need a type?
        add_node(ast, AST_IDENTIFIER, curr, true);
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_storage_class_spec_2(ParserState* s, AST* ast) {
    // TODO: special handling for typedef
    assert(is_storage_class_spec(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast, AST_STORAGE_CLASS_SPEC, s->it, false);
    ParserState_accept_it(s);
    return res;
}

static uint32_t parse_func_spec_2(ParserState* s, AST* ast) {
    assert(is_func_spec(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast, AST_FUNC_SPEC, s->it, false);
    ParserState_accept_it(s);
    return res;
}

static uint32_t parse_type_qual_2(ParserState* s, AST* ast) {
    assert(is_type_qual(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast, AST_TYPE_QUAL, s->it, false);
    ParserState_accept_it(s);
    return res;
}

static bool current_is_type_qual(ParserState* s) {
    if (is_type_qual(ParserState_curr_kind(s))) {
        if (ParserState_curr_kind(s) == TOKEN_ATOMIC) {
            return ParserState_next_token_kind(s) != TOKEN_LBRACKET;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

static bool is_declaration_spec(ParserState* s) {
    const TokenKind curr_kind = ParserState_curr_kind(s);
    return is_storage_class_spec(curr_kind) || current_is_type_qual(s)
           || is_type_spec(s) || is_func_spec(curr_kind)
           || curr_kind == TOKEN_ALIGNAS;
}

static uint32_t parse_type_spec_2(ParserState* s, AST* ast);
static uint32_t parse_align_spec_2(ParserState* s, AST* ast);

static uint32_t parse_declaration_spec_2(ParserState* s, AST* ast) {
    // TODO: maybe use parse_type_spec_qual_2() here
    assert(is_declaration_spec(s));
    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (is_storage_class_spec(curr_kind)) {
        return parse_storage_class_spec_2(s, ast);
    } else if (is_type_spec(s)) {
        return parse_type_spec_2(s, ast);
    } else if (is_type_qual(curr_kind)) {
        return parse_type_qual_2(s, ast);
    } else if (curr_kind == TOKEN_ALIGNAS) {
        return parse_align_spec_2(s, ast);
    } else if (is_func_spec(curr_kind)) {
        return parse_func_spec_2(s, ast);
    } else {
        UNREACHABLE();
    }
}

static uint32_t parse_declaration_specs_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_DECLARATION_SPECS, s->it, false);
    CHECK_ERR(parse_declaration_spec_2(s, ast));

    while (is_declaration_spec(s)) {
        CHECK_ERR(parse_declaration_spec_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_attribute_spec_sequence_2(s, ast);
        assert(rhs == ast->datas[res].rhs);
        UNUSED(rhs);
    }
    return res;
}

static uint32_t parse_attrs_and_declaration_specs_2(ParserState* s, AST* ast) {
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        const uint32_t res = add_node(ast,
                                      AST_ATTRS_AND_DECLARATION_SPECS,
                                      s->it,
                                      false);
        // TODO: cond may not be sufficient
        if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
            CHECK_ERR(parse_attribute_spec_sequence_2(s, ast));
        }

        const uint32_t rhs = parse_declaration_specs_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
        return res;
    } else {
        return parse_declaration_specs_2(s, ast);
    }
}

static uint32_t parse_pointer_2(ParserState* s, AST* ast);
static uint32_t parse_direct_declarator_2(ParserState* s, AST* ast);
static uint32_t parse_direct_abs_declarator_2(ParserState* s, AST* ast);
static uint32_t parse_abs_arr_or_func_suffix_list_2(ParserState* s, AST* ast);
static uint32_t parse_arr_or_func_suffix_list_2(ParserState* s, AST* ast);

static uint32_t parse_abs_decl_or_decl_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_TRANSLATION_UNIT, s->it, false);

    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        CHECK_ERR(parse_pointer_2(s, ast));
    }

    uint32_t rhs;
    switch (ParserState_curr_kind(s)) {
        case TOKEN_IDENTIFIER:
            ast->kinds[res] = AST_DECLARATOR;
            rhs = parse_direct_declarator_2(s, ast);
            CHECK_ERR(rhs);
            break;
        case TOKEN_LINDEX:
            ast->kinds[res] = AST_ABS_DECLARATOR;
            rhs = parse_direct_abs_declarator_2(s, ast);
            CHECK_ERR(rhs);
            break;
        case TOKEN_LBRACKET: {
            // direct_abs_declarator or direct_declarator
            rhs = add_node(ast, AST_TRANSLATION_UNIT, s->it, false);
            ParserState_accept_it(s);
            // lhs of rhs
            const uint32_t bracket_decl = parse_abs_decl_or_decl_2(s, ast);
            CHECK_ERR(bracket_decl);
            uint32_t internal_rhs;
            if (ast->kinds[bracket_decl] == AST_ABS_DECLARATOR) {
                ast->kinds[res] = AST_ABS_DECLARATOR;
                ast->kinds[rhs] = AST_DIRECT_ABS_DECLARATOR;
                CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
                internal_rhs = parse_abs_arr_or_func_suffix_list_2(s, ast);
            } else {
                ast->kinds[res] = AST_DECLARATOR;
                ast->kinds[rhs] = AST_DIRECT_DECLARATOR;
                CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
                internal_rhs = parse_arr_or_func_suffix_list_2(s, ast);
            }
            CHECK_ERR(internal_rhs);
            ast->datas[rhs].rhs = internal_rhs;
            assert(ast->kinds[rhs] != AST_TRANSLATION_UNIT);
            break;
        }
        default:
            ast->kinds[res] = AST_ABS_DECLARATOR;
            // no pointer
            if (ast->len == res + 1) {
                ParserErr_set(s->err, PARSER_ERR_EMPTY_DIRECT_ABS_DECL, s->it);
                return 0;
            }
            rhs = 0;
            break;
    }

    ast->datas[res].rhs = rhs;
    assert(ast->kinds[res] != AST_TRANSLATION_UNIT);
    return res;
}

static uint32_t parse_param_declaration_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_PARAM_DECLARATION, s->it, false);
    CHECK_ERR(parse_attrs_and_declaration_specs_2(s, ast));
    const TokenKind curr = ParserState_curr_kind(s);
    // no declarator
    if (curr == TOKEN_COMMA || curr == TOKEN_RBRACKET) {
        return res;
    }
    const uint32_t rhs = parse_abs_decl_or_decl_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_param_type_list_2(ParserState* s, AST* ast) {
    if (ParserState_curr_kind(s) == TOKEN_ELLIPSIS) {
        ParserState_accept_it(s);
        const uint32_t res = add_node(ast,
                                      AST_PARAM_TYPE_LIST_VARIADIC,
                                      s->it,
                                      false);
        ast->datas[res].rhs = ast->len;
        return res;
    }

    const uint32_t res = add_node(ast, AST_PARAM_TYPE_LIST, s->it, false);
    CHECK_ERR(parse_param_declaration_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);
        if (ParserState_curr_kind(s) == TOKEN_ELLIPSIS) {
            ParserState_accept_it(s);
            ast->kinds[res] = AST_PARAM_TYPE_LIST_VARIADIC;
            break;
        }

        CHECK_ERR(parse_param_declaration_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_func_suffix_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET);
    const uint32_t idx = s->it;
    ParserState_accept_it(s);
    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_RBRACKET) {
        ParserState_accept_it(s);
        return add_node(ast, AST_FUNC_SUFFIX, idx, false);
    } else if (curr_kind == TOKEN_IDENTIFIER
               && !ParserState_is_typedef(s, ParserState_curr_spell(s))) {
        const uint32_t res = add_node(ast, AST_FUNC_SUFFIX_OLD, idx, false);
        CHECK_ERR(parse_identifier_list_2(s, ast));
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
        return res;
    } else {
        const uint32_t res = add_node(ast, AST_FUNC_SUFFIX, idx, false);
        CHECK_ERR(parse_param_type_list_2(s, ast));
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
        return res;
    }
}

static uint32_t parse_arr_or_func_suffix_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACE
           || ParserState_curr_kind(s) == TOKEN_RBRACE);
    const uint32_t res = add_node(ast, AST_ARR_OR_FUNC_SUFFIX, s->it, false);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_LBRACE:
            CHECK_ERR(parse_arr_suffix_2(s, ast));
            break;
        case TOKEN_LBRACKET:
            CHECK_ERR(parse_func_suffix_2(s, ast));
            break;
        default:
            UNREACHABLE();
    }

    if (ParserState_curr_kind(s) == TOKEN_LINDEX
        && ParserState_next_token_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_attribute_spec_sequence_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_arr_or_func_suffix_list_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET
           || ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t res = add_node(ast,
                                  AST_ARR_OR_FUNC_SUFFIX_LIST,
                                  s->it,
                                  false);

    CHECK_ERR(parse_arr_or_func_suffix_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_LBRACKET
           || ParserState_curr_kind(s) == TOKEN_LINDEX) {
        CHECK_ERR(parse_arr_or_func_suffix_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_declarator_2(ParserState* s, AST* ast);

static uint32_t parse_direct_declarator_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_DIRECT_DECLARATOR, s->it, false);
    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_IDENTIFIER) {
        CHECK_ERR(parse_id_attribute_2(s, ast));
    } else if (curr_kind == TOKEN_LBRACKET) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_declarator_2(s, ast));
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
    } else {
        static const TokenKind ex[] = {
            TOKEN_IDENTIFIER,
            TOKEN_LBRACKET,
        };
        expected_tokens_error(s, ex, ARR_LEN(ex));
        return 0;
    }

    if (ParserState_curr_kind(s) == TOKEN_LBRACKET
        || ParserState_curr_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_arr_or_func_suffix_list_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_declarator_2(ParserState* s, AST* ast) {
    // TODO: unsure about whether this needs a type
    const uint32_t res = add_node(ast, AST_DECLARATOR, s->it, true);
    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        CHECK_ERR(parse_pointer_2(s, ast));
    }

    const uint32_t rhs = parse_direct_declarator_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_member_declarator_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_MEMBER_DECLARATOR, s->it, false);

    if (ParserState_curr_kind(s) != TOKEN_COLON) {
        CHECK_ERR(parse_declarator_2(s, ast));
    }

    if (ParserState_curr_kind(s) == TOKEN_COLON) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_const_expr_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_member_declarator_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast,
                                  AST_MEMBER_DECLARATOR_LIST,
                                  s->it,
                                  false);
    CHECK_ERR(parse_member_declarator_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        CHECK_ERR(parse_member_declarator_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_spec_qual_list_2(ParserState* s, AST* ast);

static uint32_t parse_member_declaration_body_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast,
                                  AST_MEMBER_DECLARATION_BODY,
                                  s->it,
                                  false);
    CHECK_ERR(parse_spec_qual_list_2(s, ast));

    if (ParserState_curr_kind(s) == TOKEN_SEMICOLON) {
        ParserState_accept_it(s);
        return res;
    }

    const uint32_t rhs = parse_member_declarator_list_2(s, ast);
    CHECK_ERR(rhs);
    CHECK_ERR(ParserState_accept(s, TOKEN_SEMICOLON));
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_member_declaration_2(ParserState* s, AST* ast) {
    if (ParserState_curr_kind(s) == TOKEN_STATIC_ASSERT) {
        return parse_static_assert_declaration_2(s, ast);
    }

    const uint32_t res = add_node(ast, AST_MEMBER_DECLARATION, s->it, false);
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        CHECK_ERR(parse_attribute_spec_sequence_2(s, ast));
    }

    const uint32_t rhs = parse_member_declaration_body_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_member_declaration_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast,
                                  AST_MEMBER_DECLARATION_LIST,
                                  s->it,
                                  false);
    CHECK_ERR(parse_member_declaration_2(s, ast));

    // TODO: condition may be wrong
    while (ParserState_curr_kind(s) != TOKEN_RBRACE) {
        CHECK_ERR(parse_member_declaration_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_struct_union_body_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_STRUCT_UNION_BODY, s->it, false);
    if (ParserState_curr_kind(s) == TOKEN_IDENTIFIER) {
        add_node(ast, AST_IDENTIFIER, s->it, false);
    }

    if (ParserState_curr_kind(s) == TOKEN_LBRACE) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_member_declaration_list_2(s, ast);
        CHECK_ERR(rhs);
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACE));
    }

    // neither lhs or rhs exists
    if (ast->len == res + 1) {
        // TODO: error
        return 0;
    }
    return res;
}

static uint32_t parse_struct_union_spec_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_STRUCT
           || ParserState_curr_kind(s) == TOKEN_UNION);
    const ASTNodeKind kind = ParserState_curr_kind(s) == TOKEN_STRUCT
                                 ? AST_STRUCT_SPEC
                                 : AST_UNION_SPEC;
    const uint32_t res = add_node(ast, kind, s->it, false);
    ParserState_accept_it(s);
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        CHECK_ERR(parse_attribute_spec_sequence_2(s, ast));
    }

    const uint32_t rhs = parse_struct_union_body_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_attribute_id_2(ParserState* s, AST* ast, bool* has_id) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX
           || ParserState_curr_kind(s) == TOKEN_IDENTIFIER);
    const uint32_t res = add_node(ast, AST_ATTRIBUTE_ID, s->it, false);
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        CHECK_ERR(parse_attribute_spec_sequence_2(s, ast));
    }

    if (ParserState_curr_kind(s) == TOKEN_IDENTIFIER) {
        const uint32_t rhs = add_node(ast, AST_IDENTIFIER, s->it, false);
        ParserState_accept_it(s);
        *has_id = true;
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_enum_constant_and_attribute_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast,
                                  AST_ENUM_CONSTANT_AND_ATTRIBUTE,
                                  s->it,
                                  false);
    const uint32_t const_idx = s->it;
    const StrBuf* spell = ParserState_curr_spell_buf(s);
    CHECK_ERR(ParserState_accept(s, TOKEN_IDENTIFIER));
    add_node(ast, AST_ENUM_CONSTANT, const_idx, true);
    CHECK_ERR(ParserState_register_enum_constant(s, spell, const_idx));
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_attribute_spec_sequence_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_enumerator_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_ENUMERATOR, s->it, false);
    CHECK_ERR(parse_enum_constant_and_attribute_2(s, ast));

    if (ParserState_curr_kind(s) == TOKEN_ASSIGN) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_const_expr_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_enum_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_ENUM_LIST, s->it, false);
    CHECK_ERR(parse_enumerator_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_COMMA
           && ParserState_next_token_kind(s) != TOKEN_RBRACE) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_enumerator_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_enum_body_2(ParserState* s, AST* ast, bool has_id) {
    assert(ParserState_curr_kind(s) == TOKEN_COLON
           || ParserState_curr_kind(s) == TOKEN_LBRACE);
    const uint32_t res = add_node(ast, AST_ENUM_BODY, s->it, false);
    if (ParserState_curr_kind(s) == TOKEN_COLON) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_spec_qual_list_2(s, ast));
    }

    if (ParserState_curr_kind(s) == TOKEN_LBRACE) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_enum_list_2(s, ast);
        CHECK_ERR(rhs);
        if (ParserState_curr_kind(s) == TOKEN_COMMA) {
            ParserState_accept_it(s);
        }
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACE));
        ast->datas[res].rhs = rhs;
    } else if (!has_id) {
        expected_token_error(s, TOKEN_LBRACE);
        return 0;
    }

    return res;
}

static uint32_t parse_enum_spec_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_ENUM);
    const uint32_t res = add_node(ast, AST_ENUM_SPEC, s->it, false);
    ParserState_accept_it(s);
    bool has_id = false;
    const TokenKind first_kind = ParserState_curr_kind(s);
    if (first_kind == TOKEN_LINDEX || first_kind == TOKEN_IDENTIFIER) {
        CHECK_ERR(parse_attribute_id_2(s, ast, &has_id));
    }

    const TokenKind next_kind = ParserState_curr_kind(s);
    if (next_kind == TOKEN_COLON || next_kind == TOKEN_LBRACE) {
        const uint32_t rhs = parse_enum_body_2(s, ast, has_id);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }

    return res;
}

static uint32_t parse_type_spec_2(ParserState* s, AST* ast) {
    assert(is_type_spec(s));
    const TokenKind curr_kind = ParserState_curr_kind(s);
    const uint32_t start_idx = s->it;
    switch (curr_kind) {
        case TOKEN_VOID:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_VOID, start_idx, false);
        case TOKEN_CHAR:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_CHAR, start_idx, false);
        case TOKEN_SHORT:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_SHORT, start_idx, false);
        case TOKEN_INT:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_INT, start_idx, false);
        case TOKEN_LONG:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_LONG, start_idx, false);
        case TOKEN_FLOAT:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_FLOAT, start_idx, false);
        case TOKEN_DOUBLE:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_DOUBLE, start_idx, false);
        case TOKEN_SIGNED:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_SIGNED, start_idx, false);
        case TOKEN_UNSIGNED:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_UNSIGNED, start_idx, false);
        case TOKEN_BOOL:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_BOOL, start_idx, false);
        case TOKEN_COMPLEX:
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_COMPLEX, start_idx, false);
        // TODO: _BitInt, _Decimal(32, 64, 128)
        case TOKEN_ATOMIC:
            return parse_atomic_type_spec_2(s, ast);
        case TOKEN_STRUCT:
        case TOKEN_UNION:
            return parse_struct_union_spec_2(s, ast);
        case TOKEN_ENUM:
            return parse_enum_spec_2(s, ast);
        case TOKEN_IDENTIFIER:
            // TODO: this error might not be necessary
            if (!ParserState_is_typedef(s, ParserState_curr_spell(s))) {
                ParserErr_set(s->err,
                              PARSER_ERR_EXPECTED_TYPEDEF_NAME,
                              start_idx);
                return 0;
            }
            ParserState_accept_it(s);
            return add_node(ast, AST_TYPE_SPEC_TYPEDEF_NAME, start_idx, false);
        // TODO: typeof, typeof_unqual
        default:
            UNREACHABLE();
    }
}

static uint32_t parse_align_spec_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_ALIGNAS);
    const uint32_t res = add_node(ast, AST_TRANSLATION_UNIT, s->it, false);
    ParserState_accept_it(s);
    const bool is_type_name = next_is_type_name(s);
    CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));
    uint32_t rhs;
    if (is_type_name) {
        rhs = parse_type_name_2(s, ast);
    } else {
        rhs = parse_const_expr_2(s, ast);
    }
    CHECK_ERR(rhs);

    CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_type_spec_qual_2(ParserState* s, AST* ast) {
    assert(is_type_spec(s) || is_type_qual(ParserState_curr_kind(s))
           || ParserState_curr_kind(s) == TOKEN_ALIGNAS);
    if (is_type_spec(s)) {
        return parse_type_spec_2(s, ast);
    } else if (is_type_qual(ParserState_curr_kind(s))) {
        return add_node(ast, AST_TYPE_QUAL, s->it, false);
    } else if (ParserState_curr_kind(s) == TOKEN_ALIGNAS) {
        return parse_align_spec_2(s, ast);
    } else {
        static const TokenKind ex[] = {
            // type_spec
            TOKEN_VOID,
            TOKEN_CHAR,
            TOKEN_SHORT,
            TOKEN_INT,
            TOKEN_LONG,
            TOKEN_FLOAT,
            TOKEN_DOUBLE,
            TOKEN_SIGNED,
            TOKEN_UNSIGNED,
            // TODO: _BitInt
            TOKEN_BOOL,
            TOKEN_COMPLEX,
            // TODO: Decimal(32, 64, 128)
            TOKEN_ATOMIC,
            TOKEN_STRUCT,
            TOKEN_UNION,
            TOKEN_ENUM,
            TOKEN_IDENTIFIER, // typedef name
            // TODO: typeof and typeof_unqual
            // type_qual
            TOKEN_CONST,
            TOKEN_RESTRICT,
            TOKEN_VOLATILE,
            TOKEN_ATOMIC,
            // func_spec
            TOKEN_INLINE,
            TOKEN_NORETURN,
        };
        expected_tokens_error(s, ex, ARR_LEN(ex));
        return 0;
    }
}

static uint32_t parse_spec_qual_list_without_attrs_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_SPEC_QUAL_LIST, s->it, false);
    CHECK_ERR(parse_type_spec_qual_2(s, ast));

    while (is_type_spec(s) || is_type_qual(ParserState_curr_kind(s))
           || ParserState_curr_kind(s) == TOKEN_ALIGNAS) {
        CHECK_ERR(parse_type_spec_qual_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_spec_qual_list_2(ParserState* s, AST* ast) {
    // TODO: this may not be needed
    const uint32_t res = add_node(ast, AST_SPEC_QUAL_LIST_ATTR, s->it, false);
    CHECK_ERR(parse_spec_qual_list_without_attrs_2(s, ast));

    // TODO: might need more than that to check
    if (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_attribute_spec_sequence_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_attribute_2(ParserState* s, AST* ast) {
    (void)s;
    (void)ast;
    // TODO:
    return 0;
}

static uint32_t parse_attribute_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_ATTRIBUTE_LIST, s->it, false);
    if (ParserState_curr_kind(s) == TOKEN_RINDEX) {
        ast->datas[res].rhs = ast->len;
        return res;
    }

    while (ParserState_curr_kind(s) != TOKEN_RINDEX) {
        CHECK_ERR(parse_attribute_2(s, ast));
    }

    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_attribute_spec_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX);
    ParserState_accept_it(s);
    CHECK_ERR(ParserState_accept(s, TOKEN_LINDEX));

    const uint32_t res = parse_attribute_list_2(s, ast);
    CHECK_ERR(res);

    CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX)
              && ParserState_accept(s, TOKEN_RINDEX));

    return res;
}

static uint32_t parse_attribute_spec_sequence_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t res = add_node(ast,
                                  AST_ATTRIBUTE_SPEC_SEQUENCE,
                                  s->it,
                                  false);
    CHECK_ERR(parse_attribute_spec_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_LINDEX) {
        CHECK_ERR(parse_attribute_spec_2(s, ast));
    }

    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_token_range(ParserState* s,
                                  AST* ast,
                                  ASTNodeKind kind,
                                  bool (*pred)(TokenKind)) {
    assert(pred(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast, kind, s->it, false);
    ParserState_accept_it(s);
    uint32_t len = 1;
    while (pred(ParserState_curr_kind(s))) {
        ParserState_accept_it(s);
        ++len;
    }
    // here rhs is not a node index, but how many tokens from token_idx are part
    // of the range
    ast->datas[res].rhs = len;
    return res;
}

static uint32_t parse_type_qual_list_2(ParserState* s, AST* ast) {
    return parse_token_range(s, ast, AST_TYPE_QUAL_LIST, is_type_qual);
}

static uint32_t parse_pointer_attrs_and_quals_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX
           || is_type_qual(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast,
                                  AST_POINTER_ATTRS_AND_QUALS,
                                  s->it,
                                  false);
    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        CHECK_ERR(parse_attribute_spec_sequence_2(s, ast));
    }

    if (is_type_qual(ParserState_curr_kind(s))) {
        const uint32_t rhs = parse_type_qual_list_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }

    return res;
}

static uint32_t parse_pointer_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_ASTERISK);
    const uint32_t res = add_node(ast, AST_POINTER, s->it, false);
    ParserState_accept_it(s);

    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_LINDEX || is_type_qual(curr_kind)) {
        CHECK_ERR(parse_pointer_attrs_and_quals_2(s, ast));
    }

    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        const uint32_t rhs = parse_pointer_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }

    return res;
}

static uint32_t parse_abs_arr_suffix_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t idx = s->it;
    ParserState_accept_it(s);
    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_STATIC) {
        ParserState_accept_it(s);
        const uint32_t res = add_node(ast, AST_ABS_ARR_SUFFIX_STATIC, idx, false);
        if (is_type_qual(ParserState_curr_kind(s))) {
            CHECK_ERR(parse_type_qual_list_2(s, ast));
        }
        const uint32_t rhs = parse_assign_expr_2(s, ast);
        CHECK_ERR(rhs);

        CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
        ast->datas[res].rhs = rhs;
        return res;
    } else if (curr_kind == TOKEN_ASTERISK) {
        ParserState_accept_it(s);
        CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
        return add_node(ast, AST_ABS_ARR_SUFFIX_ASTERISK, idx, false);
    } else if (is_type_qual(curr_kind)) {
        const uint32_t res = add_node(ast, AST_ABS_ARR_SUFFIX, idx, false);
        CHECK_ERR(parse_type_qual_list_2(s, ast));

        switch (ParserState_curr_kind(s)) {
            case TOKEN_RINDEX: {
                ParserState_accept_it(s);
                return res;
            }            
            case TOKEN_STATIC:
                ast->kinds[res] = AST_ABS_ARR_SUFFIX_STATIC;
                ParserState_accept_it(s);
                FALLTHROUGH();
            default: {
                const uint32_t rhs = parse_assign_expr_2(s, ast);
                CHECK_ERR(rhs);
                CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
                ast->datas[res].rhs = rhs;
                return res;
            }
        }
    } else if (curr_kind == TOKEN_RINDEX) {
        ParserState_accept_it(s);
        return add_node(ast, AST_ABS_ARR_SUFFIX, idx, false);
    } else {
        const uint32_t res = add_node(ast, AST_ARR_SUFFIX, idx, false);
        const uint32_t rhs = parse_assign_expr_2(s, ast);
        CHECK_ERR(rhs);
        CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
        ast->datas[res].rhs = rhs;
        return res;
    }
}

static uint32_t parse_abs_arr_or_func_suffix_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET || ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t res = add_node(ast, AST_ABS_ARR_OR_FUNC_SUFFIX, s->it, false);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_LBRACE:
            CHECK_ERR(parse_abs_arr_suffix_2(s, ast));
            break;
        case TOKEN_LBRACKET:
            CHECK_ERR(parse_func_suffix_2(s, ast));
            break;
        default:
            UNREACHABLE();
    }

    if (ParserState_curr_kind(s) == TOKEN_LINDEX && ParserState_next_token_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_attribute_spec_sequence_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_abs_arr_or_func_suffix_list_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET || ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t res = add_node(ast, AST_ABS_ARR_OR_FUNC_SUFFIX_LIST, s->it, false);
    CHECK_ERR(parse_abs_arr_or_func_suffix_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_LBRACKET || ParserState_curr_kind(s) == TOKEN_LINDEX) {
        CHECK_ERR(parse_abs_arr_or_func_suffix_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_abs_declarator_2(ParserState* s, AST* ast);

static uint32_t parse_direct_abs_declarator_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET
           || ParserState_curr_kind(s) == TOKEN_LINDEX);
    const uint32_t res = add_node(ast, AST_DIRECT_ABS_DECLARATOR, s->it, false);
    const TokenKind curr_kind = ParserState_curr_kind(s);
    const TokenKind next_kind = ParserState_next_token_kind(s);
    if (curr_kind == TOKEN_LBRACKET
        && (next_kind == TOKEN_ASTERISK || next_kind == TOKEN_LBRACKET
            || next_kind == TOKEN_LINDEX)) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_abs_declarator_2(s, ast));
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
    }

    if (ParserState_curr_kind(s) == TOKEN_LBRACKET
        || ParserState_curr_kind(s) == TOKEN_LINDEX) {
        const uint32_t rhs = parse_abs_arr_or_func_suffix_list_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_abs_declarator_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_ABS_DECLARATOR, s->it, false);
    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        CHECK_ERR(parse_pointer_2(s, ast));
    }

    const TokenKind curr_kind = ParserState_curr_kind(s);
    if (curr_kind == TOKEN_LBRACKET || curr_kind == TOKEN_LINDEX) {
        const uint32_t rhs = parse_direct_abs_declarator_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }

    // If the ast len has not changed, lhs and rhs are empty
    if (ast->len == res + 1) {
        ParserErr_set(s->err, PARSER_ERR_EMPTY_DIRECT_ABS_DECL, s->it);
        return 0;
    }

    return res;
}

static uint32_t parse_type_name_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_TYPE_NAME, s->it, false);
    CHECK_ERR(parse_spec_qual_list_2(s, ast));

    const TokenKind kind = ParserState_curr_kind(s);
    if (kind == TOKEN_ASTERISK || kind == TOKEN_LBRACKET
        || kind == TOKEN_LINDEX) {
        const uint32_t rhs = parse_abs_declarator_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_generic_assoc_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_GENERIC_ASSOC, s->it, true);
    if (ParserState_curr_kind(s) == TOKEN_DEFAULT) {
        ParserState_accept_it(s);
    } else {
        CHECK_ERR(parse_type_name_2(s, ast));
    }

    CHECK_ERR(ParserState_accept(s, TOKEN_COLON));

    const uint32_t rhs = parse_assign_expr_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_generic_assoc_list_2(ParserState* s, AST* ast) {
    // TODO: this probably does not need a node
    const uint32_t res = add_node(ast, AST_GENERIC_ASSOC_LIST, s->it, true);

    CHECK_ERR(parse_generic_assoc_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_generic_assoc_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_generic_sel_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_GENERIC);
    const uint32_t res = add_node(ast, AST_GENERIC_SEL, s->it, true);
    ParserState_accept_it(s);
    CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));
    // lhs
    CHECK_ERR(parse_assign_expr_2(s, ast));

    CHECK_ERR(ParserState_accept(s, TOKEN_COMMA));

    const uint32_t rhs = parse_generic_assoc_list_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_expr_2(ParserState* s, AST* ast);

static uint32_t parse_primary_expr_2(ParserState* s, AST* ast) {
    // TODO: this might not need a node
    const uint32_t res = add_node(ast, AST_PRIMARY_EXPR, s->it, true);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_IDENTIFIER:
            if (ParserState_is_enum_constant(s, ParserState_curr_spell(s))) {
                add_node(ast, AST_ENUM_CONSTANT, s->it, true);
            } else {
                add_node(ast, AST_IDENTIFIER, s->it, true);
            }
            ParserState_accept_it(s);
            break;
        case TOKEN_I_CONSTANT:
        case TOKEN_F_CONSTANT:
            add_node(ast, AST_CONSTANT, s->it, true);
            ParserState_accept_it(s);
            break;
        case TOKEN_STRING_LITERAL:
            add_node(ast, AST_STRING_LITERAL, s->it, true);
            ParserState_accept_it(s);
            break;
        case TOKEN_FUNC_NAME:
            add_node(ast, AST_FUNC, s->it, true);
            ParserState_accept_it(s);
            break;
        case TOKEN_LBRACKET:
            ParserState_accept_it(s);
            CHECK_ERR(parse_expr_2(s, ast));
            CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
            break;
        case TOKEN_GENERIC:
            CHECK_ERR(parse_generic_sel_2(s, ast));
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
    return parse_token_range(s,
                             ast,
                             AST_STORAGE_CLASS_SPECS,
                             is_storage_class_spec);
}

static uint32_t parse_compound_literal_type_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_COMPOUND_LITERAL_TYPE, s->it, false);
    if (is_storage_class_spec(ParserState_curr_kind(s))) {
        CHECK_ERR(parse_storage_class_specs_2(s, ast));
    }

    const uint32_t rhs = parse_type_name_2(s, ast);
    CHECK_ERR(rhs);
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

static uint32_t parse_designator_2(ParserState* s, AST* ast) {
    assert(is_designator(ParserState_curr_kind(s)));
    // TODO: might not need node
    const uint32_t res = add_node(ast, AST_DESIGNATOR, s->it, true);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_LINDEX:
            ParserState_accept_it(s);
            CHECK_ERR(parse_const_expr_2(s, ast));
            CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
            break;
        case TOKEN_DOT:
            ParserState_accept_it(s);
            const uint32_t id_idx = s->it;
            CHECK_ERR(ParserState_accept(s, TOKEN_IDENTIFIER));
            add_node(ast, AST_IDENTIFIER, id_idx, true);
            break;
        default:
            UNREACHABLE();
    }
    return res;
}

static uint32_t parse_designator_list_2(ParserState* s, AST* ast) {
    assert(is_designator(ParserState_curr_kind(s)));
    const uint32_t res = add_node(ast, AST_DESIGNATOR_LIST, s->it, false);
    CHECK_ERR(parse_designator_2(s, ast));

    while (is_designator(ParserState_curr_kind(s))) {
        CHECK_ERR(parse_designator_2(s, ast));
    }

    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_braced_initializer_2(ParserState* s, AST* ast);

static uint32_t parse_initializer_2(ParserState* s, AST* ast) {
    // TODO: Might not need a node
    const uint32_t res = add_node(ast, AST_INITIALIZER, s->it, false);
    if (ParserState_curr_kind(s) == TOKEN_RBRACE) {
        CHECK_ERR(parse_braced_initializer_2(s, ast));
    } else {
        CHECK_ERR(parse_assign_expr_2(s, ast));
    }
    return res;
}

static uint32_t parse_designation_init_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_DESIGNATION_INIT, s->it, false);
    if (is_designator(ParserState_curr_kind(s))) {
        CHECK_ERR(parse_designator_list_2(s, ast));

        CHECK_ERR(ParserState_accept(s, TOKEN_ASSIGN));
    }

    const uint32_t rhs = parse_initializer_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_init_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_INIT_LIST, s->it, false);
    CHECK_ERR(parse_designation_init_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_COMMA
           && ParserState_next_token_kind(s) != TOKEN_RBRACE) {
        ParserState_accept_it(s);

        CHECK_ERR(parse_designation_init_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
    return res;
}

static uint32_t parse_braced_initializer_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_BRACED_INITIALIZER, s->it, false);
    CHECK_ERR(ParserState_accept(s, TOKEN_LBRACE));

    if (ParserState_curr_kind(s) != TOKEN_RBRACE) {
        const uint32_t rhs = parse_init_list_2(s, ast);
        CHECK_ERR(rhs);

        if (ParserState_curr_kind(s) == TOKEN_COMMA) {
            ParserState_accept_it(s);
        }
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACE));
        ast->datas[res].rhs = rhs;
        return res;
    } else {
        ParserState_accept_it(s);
        return res;
    }
}

static uint32_t parse_compound_literal_2(ParserState* s, AST* ast) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET);
    const uint32_t res = add_node(ast, AST_COMPOUND_LITERAL, s->it, true);
    ParserState_accept_it(s);
    CHECK_ERR(parse_compound_literal_type_2(s, ast));
    CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));

    const uint32_t rhs = parse_braced_initializer_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_arg_expr_list_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_ARG_EXPR_LIST, s->it, false);
    // list is empty
    if (ParserState_curr_kind(s) == TOKEN_RBRACKET) {
        ast->datas[res].rhs = ast->len;
        return res;
    }

    CHECK_ERR(parse_assign_expr_2(s, ast));

    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);

        CHECK_ERR(parse_assign_expr_2(s, ast));
    }
    ast->datas[res].rhs = ast->len;
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

static uint32_t parse_postfix_expr_2(ParserState* s, AST* ast) {
    // TODO: might not need node
    const uint32_t res = add_node(ast, AST_POSTFIX_EXPR, s->it, true);

    if (ParserState_curr_kind(s) == TOKEN_LBRACKET && next_is_type_name(s)) {
        CHECK_ERR(parse_compound_literal_2(s, ast));
    } else {
        CHECK_ERR(parse_primary_expr_2(s, ast));
    }

    uint32_t curr_node_idx = res;
    while (is_postfix_op(ParserState_curr_kind(s))) {
        const uint32_t rhs = add_node(ast, AST_TRANSLATION_UNIT, s->it, true);
        const TokenKind curr_kind = ParserState_curr_kind(s);
        switch (curr_kind) {
            case TOKEN_LINDEX:
                ast->kinds[rhs] = AST_POSTFIX_OP_INDEX;
                ParserState_accept_it(s);
                CHECK_ERR(parse_expr_2(s, ast));
                CHECK_ERR(ParserState_accept(s, TOKEN_RINDEX));
                break;
            case TOKEN_LBRACKET:
                ast->kinds[rhs] = AST_POSTFIX_OP_CALL;
                ParserState_accept_it(s);
                CHECK_ERR(parse_arg_expr_list_2(s, ast));
                CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
                break;
            case TOKEN_DOT:
            case TOKEN_PTR_OP:
                ast->kinds[rhs] = curr_kind == TOKEN_DOT
                                      ? AST_POSTFIX_OP_ACCESS
                                      : AST_POSTFIX_OP_PTR_ACCESS;
                ParserState_accept_it(s);
                const uint32_t id_idx = s->it;
                CHECK_ERR(ParserState_accept(s, TOKEN_IDENTIFIER));
                add_node(ast, AST_IDENTIFIER, id_idx, true);
                break;
            case TOKEN_INC:
            case TOKEN_DEC:
                ast->kinds[rhs] = curr_kind == TOKEN_INC ? AST_POSTFIX_OP_INC
                                                         : AST_POSTFIX_OP_DEC;
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

static uint32_t parse_unary_expr_2(ParserState* s, AST* ast);

static uint32_t parse_cast_expr_2(ParserState* s, AST* ast) {
    // TODO: might not need a node
    const uint32_t res = add_node(ast, AST_CAST_EXPR, s->it, true);

    if (ParserState_curr_kind(s) == TOKEN_LBRACKET && next_is_type_name(s)) {
        ParserState_accept_it(s);
        CHECK_ERR(parse_type_name_2(s, ast));
        CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));

        const uint32_t rhs = parse_cast_expr_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    } else {
        CHECK_ERR(parse_unary_expr_2(s, ast));
    }
    return res;
}

static ASTNodeKind unary_op_to_unary_node_kind(TokenKind k) {
    switch (k) {
        case TOKEN_AND:
            return AST_UNARY_EXPR_ADDRESSOF;
        case TOKEN_ASTERISK:
            return AST_UNARY_EXPR_DEREF;
        case TOKEN_ADD:
            return AST_UNARY_EXPR_PLUS;
        case TOKEN_SUB:
            return AST_UNARY_EXPR_MINUS;
        case TOKEN_BNOT:
            return AST_UNARY_EXPR_BNOT;
        case TOKEN_NOT:
            return AST_UNARY_EXPR_NOT;
        default:
            UNREACHABLE();
    }
}

static uint32_t parse_unary_expr_2(ParserState* s, AST* ast) {
    const TokenKind curr_kind = ParserState_curr_kind(s);
    switch (curr_kind) {
        case TOKEN_INC:
        case TOKEN_DEC: {
            const ASTNodeKind kind = curr_kind == TOKEN_INC
                                         ? AST_UNARY_EXPR_INC
                                         : AST_UNARY_EXPR_DEC;
            const uint32_t res = add_node(ast, kind, s->it, true);
            ParserState_accept_it(s);
            CHECK_ERR(parse_unary_expr_2(s, ast));
            return res;
        }
        case TOKEN_SIZEOF: {
            const uint32_t res = add_node(ast,
                                          AST_UNARY_EXPR_SIZEOF,
                                          s->it,
                                          true);
            ParserState_accept_it(s);
            if (ParserState_curr_kind(s) == TOKEN_LBRACKET) {
                if (next_is_type_name(s)) {
                    ParserState_accept_it(s);
                    CHECK_ERR(parse_type_name_2(s, ast));
                    CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
                } else {
                    CHECK_ERR(parse_unary_expr_2(s, ast));
                }
            } else {
                CHECK_ERR(parse_unary_expr_2(s, ast));
            }
            return res;
        }
        case TOKEN_ALIGNOF: {
            const uint32_t res = add_node(ast,
                                          AST_UNARY_EXPR_ALIGNOF,
                                          s->it,
                                          true);
            ParserState_accept_it(s);
            CHECK_ERR(ParserState_accept(s, TOKEN_LBRACKET));
            CHECK_ERR(parse_type_name_2(s, ast));
            CHECK_ERR(ParserState_accept(s, TOKEN_RBRACKET));
            return res;
        }
        case TOKEN_AND:
        case TOKEN_ASTERISK:
        case TOKEN_ADD:
        case TOKEN_SUB:
        case TOKEN_BNOT:
        case TOKEN_NOT: {
            const ASTNodeKind kind = unary_op_to_unary_node_kind(curr_kind);
            const uint32_t res = add_node(ast, kind, s->it, true);
            ParserState_accept_it(s);
            CHECK_ERR(parse_cast_expr_2(s, ast));
            return res;
        }
        default:
            return parse_postfix_expr_2(s, ast);
    }
}

static uint32_t parse_binary_expr_multiple_ops(
    ParserState* s,
    AST* ast,
    ASTNodeKind default_kind,
    uint32_t (*parse_child)(ParserState*, AST*),
    ASTNodeKind (*op_to_node_kind)(TokenKind)) {
    // TODO: might not need node
    const uint32_t res = add_node(ast, default_kind, s->it, true);
    CHECK_ERR(parse_child(s, ast));

    const ASTNodeKind kind = op_to_node_kind(ParserState_curr_kind(s));
    if (kind != AST_TRANSLATION_UNIT) {
        ast->kinds[res] = kind;
        ParserState_accept_it(s);
        const uint32_t rhs = parse_binary_expr_multiple_ops(s,
                                                            ast,
                                                            default_kind,
                                                            parse_child,
                                                            op_to_node_kind);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static ASTNodeKind mul_op_to_mul_expr_kind(TokenKind k) {
    switch (k) {
        case TOKEN_ASTERISK:
            return AST_MUL_EXPR;
        case TOKEN_DIV:
            return AST_DIV_EXPR;
        case TOKEN_MOD:
            return AST_MOD_EXPR;
        default:
            return AST_TRANSLATION_UNIT;
    }
}

static uint32_t parse_mul_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_multiple_ops(s,
                                          ast,
                                          AST_MUL_EXPR,
                                          parse_cast_expr_2,
                                          mul_op_to_mul_expr_kind);
}

static ASTNodeKind add_op_to_add_expr_kind(TokenKind k) {
    switch (k) {
        case TOKEN_ADD:
            return AST_ADD_EXPR;
        case TOKEN_SUB:
            return AST_SUB_EXPR;
        default:
            return AST_TRANSLATION_UNIT;
    }
}

static uint32_t parse_add_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_multiple_ops(s,
                                          ast,
                                          AST_ADD_EXPR,
                                          parse_mul_expr_2,
                                          add_op_to_add_expr_kind);
}

static ASTNodeKind shift_op_to_shift_expr_kind(TokenKind k) {
    switch (k) {
        case TOKEN_LSHIFT:
            return AST_LSHIFT_EXPR;
        case TOKEN_RSHIFT:
            return AST_RSHIFT_EXPR;
        default:
            return AST_TRANSLATION_UNIT;
    }
}

static uint32_t parse_shift_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_multiple_ops(s,
                                          ast,
                                          AST_LSHIFT_EXPR,
                                          parse_add_expr_2,
                                          shift_op_to_shift_expr_kind);
}

static ASTNodeKind rel_op_to_rel_expr_kind(TokenKind k) {
    switch (k) {
        case TOKEN_LT:
            return AST_REL_EXPR_LT;
        case TOKEN_GT:
            return AST_REL_EXPR_GT;
        case TOKEN_LE:
            return AST_REL_EXPR_LE;
        case TOKEN_GE:
            return AST_REL_EXPR_GE;
        default:
            return AST_TRANSLATION_UNIT;
    }
}

static uint32_t parse_rel_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_multiple_ops(s,
                                          ast,
                                          AST_REL_EXPR_GT,
                                          parse_shift_expr_2,
                                          rel_op_to_rel_expr_kind);
}

static ASTNodeKind eq_op_to_eq_expr_kind(TokenKind k) {
    switch (k) {
        case TOKEN_EQ:
            return AST_EQ_EXPR;
        case TOKEN_NE:
            return AST_NE_EXPR;
        default:
            return AST_TRANSLATION_UNIT;
    }
}

static uint32_t parse_eq_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_multiple_ops(s,
                                          ast,
                                          AST_EQ_EXPR,
                                          parse_rel_expr_2,
                                          eq_op_to_eq_expr_kind);
}

static uint32_t parse_binary_expr_single_op(
    ParserState* s,
    AST* ast,
    uint32_t (*parse_child)(ParserState*, AST* ast),
    TokenKind op,
    ASTNodeKind kind) {
    const uint32_t res = add_node(ast, kind, s->it, true);
    CHECK_ERR(parse_child(s, ast));

    if (ParserState_curr_kind(s) == op) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_binary_expr_single_op(s,
                                                         ast,
                                                         parse_child,
                                                         op,
                                                         kind);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static uint32_t parse_and_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_single_op(s,
                                       ast,
                                       parse_eq_expr_2,
                                       TOKEN_AND,
                                       AST_AND_EXPR);
}

static uint32_t parse_xor_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_single_op(s,
                                       ast,
                                       parse_and_expr_2,
                                       TOKEN_XOR,
                                       AST_XOR_EXPR);
}

static uint32_t parse_or_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_single_op(s,
                                       ast,
                                       parse_xor_expr_2,
                                       TOKEN_OR,
                                       AST_OR_EXPR);
}

static uint32_t parse_log_and_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_single_op(s,
                                       ast,
                                       parse_or_expr_2,
                                       TOKEN_LAND,
                                       AST_LOG_AND_EXPR);
}

static uint32_t parse_log_or_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_single_op(s,
                                       ast,
                                       parse_log_and_expr_2,
                                       TOKEN_LOR,
                                       AST_LOG_OR_EXPR);
}

static uint32_t parse_cond_expr_2(ParserState* s, AST* ast);

static uint32_t parse_cond_items_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_COND_ITEMS, s->it, true);
    CHECK_ERR(parse_expr_2(s, ast));

    CHECK_ERR(ParserState_accept(s, TOKEN_COLON));

    const uint32_t rhs = parse_cond_expr_2(s, ast);
    CHECK_ERR(rhs);
    ast->datas[res].rhs = rhs;
    return res;
}

static uint32_t parse_cond_expr_2(ParserState* s, AST* ast) {
    const uint32_t res = add_node(ast, AST_COND_EXPR, s->it, true);
    CHECK_ERR(parse_log_or_expr_2(s, ast));

    if (ParserState_curr_kind(s) == TOKEN_QMARK) {
        ParserState_accept_it(s);
        const uint32_t rhs = parse_cond_items_2(s, ast);
        CHECK_ERR(rhs);
        ast->datas[res].rhs = rhs;
    }
    return res;
}

static ASTNodeKind assign_op_to_assign_expr_kind(TokenKind k) {
    switch (k) {
        case TOKEN_ASSIGN:
            return AST_ASSIGN;
        case TOKEN_MUL_ASSIGN:
            return AST_ASSIGN_MUL;
        case TOKEN_DIV_ASSIGN:
            return AST_ASSIGN_DIV;
        case TOKEN_MOD_ASSIGN:
            return AST_ASSIGN_MOD;
        case TOKEN_ADD_ASSIGN:
            return AST_ASSIGN_ADD;
        case TOKEN_SUB_ASSIGN:
            return AST_ASSIGN_SUB;
        case TOKEN_LSHIFT_ASSIGN:
            return AST_ASSIGN_SHL;
        case TOKEN_RSHIFT_ASSIGN:
            return AST_ASSIGN_SHR;
        case TOKEN_AND_ASSIGN:
            return AST_ASSIGN_AND;
        case TOKEN_XOR_ASSIGN:
            return AST_ASSIGN_XOR;
        case TOKEN_OR_ASSIGN:
            return AST_ASSIGN_OR;
        default:
            return AST_TRANSLATION_UNIT;
    }
}

// In the grammar it the first element is actually a unary expression, but
// because differentiating is actually not that easy, leave that error to
// contextanalysis
static uint32_t parse_assign_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_multiple_ops(s,
                                          ast,
                                          AST_ASSIGN,
                                          parse_cond_expr_2,
                                          assign_op_to_assign_expr_kind);
}

static uint32_t parse_expr_2(ParserState* s, AST* ast) {
    return parse_binary_expr_single_op(s,
                                       ast,
                                       parse_assign_expr_2,
                                       TOKEN_COMMA,
                                       AST_EXPR);
}

static uint32_t parse_const_expr_2(ParserState* s, AST* ast) {
    return parse_cond_expr_2(s, ast);
}
