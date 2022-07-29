#ifndef PARSER_H
#define PARSER_H

#include "frontend/ast/translation_unit.h"
#include "frontend/token.h"

struct translation_unit parse_tokens(struct token* tokens,
                                     struct parser_err* err);

#endif
