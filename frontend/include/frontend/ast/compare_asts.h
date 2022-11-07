#ifndef COMPARE_ASTS_H
#define COMPARE_ASTS_H

#include <stdbool.h>

#include "frontend/ast/translation_unit.h"

bool compare_asts(const struct translation_unit* tl1,
                  const struct translation_unit* tl2);

#endif

