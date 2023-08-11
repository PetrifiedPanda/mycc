#include "frontend/ast/SpecQualList.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_spec_or_qual(ParserState* s, SpecQualList* res) {
    assert(res);

    if (is_type_qual(ParserState_curr_kind(s))) {
        update_type_quals(s, &res->quals);
    } else if (!update_type_specs(s, &res->specs)) {
        return false;
    }

    return true;
}

bool parse_spec_qual_list(ParserState* s, SpecQualList* res) {
    *res = (SpecQualList){
        .info = AstNodeInfo_create(ParserState_curr_idx(s)),
        .quals = TypeQuals_create(),
        .specs = TypeSpecs_create(),
    };

    if (!parse_spec_or_qual(s, res)) {
        return false;
    }

    while (is_type_spec(s) || is_type_qual(ParserState_curr_kind(s))) {
        if (!parse_spec_or_qual(s, res)) {
            SpecQualList_free_children(res);
            return false;
        }
    }

    return res;
}

void SpecQualList_free_children(SpecQualList* l) {
    TypeSpecs_free_children(&l->specs);
}

void SpecQualList_free(SpecQualList* l) {
    SpecQualList_free_children(l);
    mycc_free(l);
}

