#include "ast/enum_spec.h"

#include <stdlib.h>

static void free_children(EnumSpec* s) {

}

void free_enum_spec(EnumSpec* s) {
    free_children(s);
    free(s);
}