#ifndef AST_SERIALIZER_H
#define AST_SERIALIZER_H

#include <stdio.h>

#include "frontend/file_info.h"

#include "translation_unit.h"

bool serialize_ast(const struct translation_unit* tl,
                   const struct file_info* file_info,
                   FILE* f);

#endif

