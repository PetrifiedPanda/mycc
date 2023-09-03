#include "frontend/parser/parser.h"

#include <assert.h>

#include "frontend/parser/ParserState.h"

#include "util/timing.h"

TranslationUnit parse_tokens(TokenArr* tokens, ParserErr* err) {
    assert(tokens);
    MYCC_TIMER_BEGIN();
    ParserState state = ParserState_create(tokens, err);
    TranslationUnit res = parse_translation_unit(&state);
    ParserState_free(&state);
    MYCC_TIMER_END("parser");
    return res;
}
