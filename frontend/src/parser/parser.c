#include "frontend/parser/parser.h"

#include <assert.h>

#include "frontend/parser/ParserState.h"

TranslationUnit parse_tokens(Token* tokens, ParserErr* err) {
    assert(tokens);

    ParserState state = create_parser_state(tokens, err);
    TranslationUnit res = parse_translation_unit(&state);
    free_parser_state(&state);
    return res;
}
