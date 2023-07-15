#ifndef MYCC_FRONTEND_AST_AST_SERIALIZER_H
#define MYCC_FRONTEND_AST_AST_SERIALIZER_H

#include "util/File.h"

#include "frontend/FileInfo.h"

#include "TranslationUnit.h"

bool serialize_ast(const TranslationUnit* tl, const FileInfo* file_info, File f);

#endif

