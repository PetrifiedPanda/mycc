#include "frontend/ast/compare_asts.h"

static bool compare_translation_units(const struct translation_unit* tl1,
                                      const struct translation_unit* tl2);

bool compare_asts(const struct translation_unit* tl1,
                  const struct translation_unit* tl2) {
    return compare_translation_units(tl1, tl2);
}

static bool compare_func_defs(const struct func_def* d1,
                              const struct func_def* d2) {
    (void)d1, (void)d2;
    // TODO:
    return false;
}

static bool compare_declarations(const struct declaration* d1,
                                 const struct declaration* d2) {
    (void)d1, (void)d2;
    // TODO:
    return false;
}

static bool compare_external_declarations(
    const struct external_declaration* d1,
    const struct external_declaration* d2) {
    if (d1->is_func_def != d2->is_func_def) {
        return false;
    }

    if (d1->is_func_def) {
        return compare_func_defs(&d1->func_def, &d2->func_def);
    } else {
        return compare_declarations(&d1->decl, &d2->decl);
    }
}

static bool compare_translation_units(const struct translation_unit* tl1,
                                      const struct translation_unit* tl2) {
    if (tl1->len != tl2->len) {
        return false;
    }

    for (size_t i = 0; i < tl1->len; ++i) {
        const struct external_declaration* d1 = &tl1->external_decls[i];
        const struct external_declaration* d2 = &tl2->external_decls[i];
        if (!compare_external_declarations(d1, d2)) {
            return false;
        }
    }
    return true;
}
