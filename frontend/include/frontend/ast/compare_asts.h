#ifndef COMPARE_ASTS_H
#define COMPARE_ASTS_H

#include <stdbool.h>

#include "TranslationUnit.h"

bool compare_asts(const TranslationUnit* tl1, const FileInfo* i1,
                  const TranslationUnit* tl2, const FileInfo* i2);
#endif

