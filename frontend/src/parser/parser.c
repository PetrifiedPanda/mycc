#include "frontend/parser/parser.h"

#include <assert.h>

#include "frontend/parser/ParserState.h"

TranslationUnit parse_tokens(TokenArr* tokens, ParserErr* err) {
    assert(tokens);

    ParserState state = ParserState_create(tokens, err);
    TranslationUnit res = parse_translation_unit(&state);
    ParserState_free(&state);
    return res;
}
