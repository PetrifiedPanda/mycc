#ifndef PARSER_H
#define PARSER_H

#include "ast/translation_unit.h"
#include "token.h"

struct translation_unit parse_tokens(struct token* tokens,
                                     struct parser_err* err);

#endif
