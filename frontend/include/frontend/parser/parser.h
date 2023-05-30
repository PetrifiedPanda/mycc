#ifndef PARSER_H
#define PARSER_H

#include "frontend/Token.h"

#include "frontend/ast/ast.h"

TranslationUnit parse_tokens(Token* tokens, ParserErr* err);

#endif
