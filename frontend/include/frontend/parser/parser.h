#ifndef MYCC_FRONTEND_PARSER_PARSER_H
#define MYCC_FRONTEND_PARSER_PARSER_H

#include "frontend/Token.h"

#include "frontend/ast/ast.h"

TranslationUnit parse_tokens(TokenArr* tokens, ParserErr* err);

#endif
