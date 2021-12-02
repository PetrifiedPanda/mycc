#ifndef PARSER_H
#define PARSER_H

#include "ast/translation_unit.h"
#include "token.h"

TranslationUnit* parse_tokens(Token* tokens);

#endif

