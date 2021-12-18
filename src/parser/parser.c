#include "parser/parser.h"

#include <assert.h>

#include "error.h"

#include "parser/parser_state.h"

struct translation_unit parse_tokens(struct token* tokens) {
    struct parser_state state = {.it = tokens};
    struct translation_unit res = parse_translation_unit(&state);
    assert(state.it->type == INVALID);
    return res;
}
