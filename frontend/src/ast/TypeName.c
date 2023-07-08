#include "frontend/ast/TypeName.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

#include "frontend/ast/declaration/AbsDeclarator.h"

#include "frontend/ast/SpecQualList.h"

bool parse_type_name_inplace(ParserState* s, TypeName* res) {
    assert(res);

    if (is_type_spec(s) || is_type_qual(ParserState_curr_kind(s))) {
        res->spec_qual_list = mycc_alloc(sizeof *res->spec_qual_list);
        if (!parse_spec_qual_list(s, res->spec_qual_list)) {
            return false;
        }
    } else {
        // might be better for the error to just say "Expected type specifier or
        // type qualifier"
        static const TokenKind expected[] = {
            TOKEN_VOID,         TOKEN_CHAR,   TOKEN_SHORT,    TOKEN_INT,
            TOKEN_LONG,         TOKEN_FLOAT,  TOKEN_DOUBLE,   TOKEN_SIGNED,
            TOKEN_UNSIGNED,     TOKEN_BOOL,   TOKEN_COMPLEX,  TOKEN_IMAGINARY,
            TOKEN_ATOMIC,       TOKEN_STRUCT, TOKEN_UNION,    TOKEN_ENUM,
            TOKEN_TYPEDEF_NAME, TOKEN_CONST,  TOKEN_RESTRICT, TOKEN_VOLATILE,
            TOKEN_ATOMIC,
        };
        expected_tokens_error(s, expected, ARR_LEN(expected));
        return false;
    }

    if (ParserState_curr_kind(s) == TOKEN_ASTERISK
        || ParserState_curr_kind(s) == TOKEN_LBRACKET
        || ParserState_curr_kind(s) == TOKEN_LINDEX) {
        res->abstract_decl = parse_abs_declarator(s);
        if (!res->abstract_decl) {
            SpecQualList_free(res->spec_qual_list);
            return false;
        }
    } else {
        res->abstract_decl = NULL;
    }

    return true;
}

TypeName* parse_type_name(ParserState* s) {
    TypeName* res = mycc_alloc(sizeof *res);
    if (!parse_type_name_inplace(s, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

void TypeName_free_children(TypeName* n) {
    SpecQualList_free(n->spec_qual_list);
    if (n->abstract_decl) {
        AbsDeclarator_free(n->abstract_decl);
    }
}

void TypeName_free(TypeName* n) {
    TypeName_free_children(n);
    mycc_free(n);
}

