#ifndef AST_BIN_DUMPER
#define AST_BIN_DUMPER

#include <stdio.h>

#include "frontend/file_info.h"

#include "translation_unit.h"

bool bin_dump_ast(const struct translation_unit* tl,
                  const struct file_info* file_info,
                  FILE* f);

#endif

