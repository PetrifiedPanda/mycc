#ifndef AST_DUMPER_H
#define AST_DUMPER_H

#include <stdio.h>

#include "frontend/file_info.h"

#include "translation_unit.h"

bool dump_ast(const struct translation_unit* tl,
              const struct file_info* file_info,
              FILE* f);

#endif

