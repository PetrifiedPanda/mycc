#ifndef READ_AND_TOKENIZE_LINE_H
#define READ_AND_TOKENIZE_LINE_H

#include <stdbool.h>

#include "frontend/preproc/PreprocState.h"

bool read_and_tokenize_line(PreprocState* state, const ArchTypeInfo* info);

#endif

