#ifndef ERR_BASE_H
#define ERR_BASE_H

#include <stdio.h>

#include "FileInfo.h"
#include "Token.h"

typedef struct {
    SourceLoc loc;
} ErrBase;

ErrBase create_err_base(SourceLoc loc);

void print_err_base(FILE* out,
                    const FileInfo* file_info,
                    const ErrBase* err);

#endif

