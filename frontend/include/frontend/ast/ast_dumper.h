#ifndef AST_DUMPER_H
#define AST_DUMPER_H

#include <stdio.h>

#include "frontend/FileInfo.h"

#include "TranslationUnit.h"

bool dump_ast(const TranslationUnit* tl, const FileInfo* file_info, FILE* f);

#endif

