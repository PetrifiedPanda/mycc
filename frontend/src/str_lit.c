#include "frontend/str_lit.h"

struct str_lit create_str_lit(enum str_lit_kind kind,
                              const struct str* contents) {
    return (struct str_lit){
        .kind = kind,
        .contents = *contents,
    };
}
