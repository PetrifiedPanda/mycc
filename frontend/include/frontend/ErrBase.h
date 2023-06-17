#ifndef ERR_BASE_H
#define ERR_BASE_H

#include "util/File.h"

#include "FileInfo.h"
#include "Token.h"

typedef struct {
    SourceLoc loc;
} ErrBase;

ErrBase ErrBase_create(SourceLoc loc);

void ErrBase_print(File out, const FileInfo* file_info, const ErrBase* err);

#endif

