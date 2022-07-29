#include "frontend/parser/parser.h"

#include <assert.h>

#include "frontend/parser/parser_state.h"

struct translation_unit parse_tokens(struct token* tokens,
                                     struct parser_err* err) {
    assert(tokens);

    struct parser_state state = create_parser_state(tokens, err);
    struct translation_unit res = parse_translation_unit(&state);
    free_parser_state(&state);
    return res;
}
