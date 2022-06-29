#ifndef AST_DUMPER_H
#define AST_DUMPER_H

#include <stdio.h>

#include "ast/translation_unit.h"

bool dump_ast(const struct translation_unit* tl, FILE* f);

#endif

