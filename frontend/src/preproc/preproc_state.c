#include "frontend/preproc/preproc_state.h"

#include <string.h>
#include <ctype.h>

#include "util/mem.h"
#include "util/file.h"
#include "util/macro_util.h"

#include "frontend/preproc/preproc_macro.h"

struct opened_file_info {
    size_t idx;
    long pos;
    struct file_loc file_loc;
};

struct file_data {
    bool is_valid;
    struct file_manager fm;
    struct file_info fi;
};

static struct file_data create_file_data(const char* start_file,
                                         struct preproc_err* err) {
    struct str file_name = create_str(strlen(start_file), start_file);
    FILE* file = fopen(start_file, "r");
    if (!file) {
        set_preproc_file_err(err, 0, (struct source_loc){0, {0, 0}}, true);
        return (struct file_data){0};
    }

    struct file_info fi = create_file_info(&file_name);
    struct file_manager fm = {
        .files = {[0] = file},
        .current_file_idx = 0,
        .opened_info = mycc_alloc(sizeof *fm.opened_info),
        .opened_info_len = 1,
        .opened_info_cap = 1,
    };
    *fm.opened_info = (struct opened_file_info){
        .idx = 0,
        .pos = -1,
        .file_loc = {0, 0},
    };
    return (struct file_data){
        .is_valid = true,
        .fm = fm,
        .fi = fi,
    };
}

struct preproc_state create_preproc_state(const char* start_file,
                                          struct preproc_err* err) {
    struct file_data fd = create_file_data(start_file, err);
    if (!fd.is_valid) {
        return (struct preproc_state){0};
    }
    return (struct preproc_state){
        .res =
            {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            },
        .line_info =
            {
                .line = create_empty_str(),
                .next = NULL,
                .curr_loc =
                    {
                        .file_idx = 0,
                        .file_loc = {0, 1},
                    },
            },
        .file_manager = fd.fm,
        .conds_len = 0,
        .conds_cap = 0,
        .conds = NULL,
        .err = err,
        ._macro_map = create_string_hash_map(
            sizeof(struct preproc_macro),
            100,
            true,
            (void (*)(void*))free_preproc_macro),
        .file_info = fd.fi,
    };
}

struct preproc_state create_preproc_state_string(const char* code,
                                                 const char* filename,
                                                 struct preproc_err* err) {
    struct str filename_str = create_str(strlen(filename), filename);
    return (struct preproc_state){
        .res =
            {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            },
        .line_info =
            {
                .line = create_empty_str(),
                .next = code,
                .curr_loc =
                    {
                        .file_idx = 0,
                        .file_loc = {1, 1},
                    },
            },
        .file_manager =
            {
                .files = {0},
                .current_file_idx = 0,
                .opened_info = NULL,
                .opened_info_len = 0,
                .opened_info_cap = 0,
            },
        .conds_len = 0,
        .conds_cap = 0,
        .conds = NULL,
        .err = err,
        ._macro_map = create_string_hash_map(
            sizeof(struct preproc_macro),
            100,
            true,
            (void (*)(void*))free_preproc_macro),
        .file_info = create_file_info(&filename_str),
    };
}

static bool is_escaped_newline(const char* line, size_t len) {
    if (len == 0) {
        return false;
    }
    const char* it = line + len - 1;
    while (isspace(*it) && it != line) {
        --it;
    }
    return *it == '\\';
}

static FILE* get_current_file(const struct preproc_state* state) {
    return state->file_manager.files[state->file_manager.current_file_idx];
}

void preproc_state_read_line(struct preproc_state* state) {
    assert(state);
    str_clear(&state->line_info.line);
    size_t len = 0;
    enum {
        STATIC_BUF_LEN = ARR_LEN(state->line_info.static_buf),
    };
    char* static_buf = state->line_info.static_buf;
    struct str* line = &state->line_info.line;
    FILE* file = get_current_file(state);
    state->line_info.next = file_read_line(file,
                                           line,
                                           &len,
                                           static_buf,
                                           STATIC_BUF_LEN);
    while (is_escaped_newline(state->line_info.next, len)) {
        if (state->line_info.next == str_get_data(line)) {
            str_push_back(line, '\n');
        } else if (len < STATIC_BUF_LEN - 1) {
            static_buf[len] = '\n';
            static_buf[len + 1] = '\0';
        } else {
            assert(str_len(line) == 0);
            str_reserve(line, len + 1);
            str_append_c_str(line, len, static_buf);
            str_push_back(line, '\n');
        }
        ++len;
        state->line_info.next = file_read_line(file,
                                               line,
                                               &len,
                                               static_buf,
                                               STATIC_BUF_LEN);
    }
    state->line_info.curr_loc.file_loc.line += 1;
    state->line_info.curr_loc.file_loc.index = 1;
}

bool preproc_state_over(const struct preproc_state* state) {
    FILE* file = get_current_file(state);
    return (state->line_info.next == NULL || *state->line_info.next == '\0')
           && (file == NULL || feof(file));
}

bool preproc_state_open_file(struct preproc_state* s,
                             const struct str* filename_str,
                             struct source_loc include_loc) {
    struct file_manager* fm = &s->file_manager;
    if (fm->current_file_idx == FOPEN_MAX - 1) {
        long pos = ftell(fm->files[0]);
        fclose(fm->files[0]);
        memmove(fm->files, fm->files + 1, sizeof fm->files - sizeof(FILE*));
        // TODO: when the index calculation is sus
        fm->opened_info[fm->opened_info_len - 1 - FOPEN_MAX - 1].pos = pos;
        --fm->current_file_idx;
    }

    // TODO: find actual file path and check if file already is in file_info
    file_info_add(&s->file_info, filename_str);
    const size_t idx = s->file_info.len - 1;
    const char* filename = str_get_data(&s->file_info.paths[idx]);
    FILE* file = fopen(filename, "r");
    if (!file) {
        set_preproc_err(s->err, 0, include_loc);
        return false;
    }
    ++fm->current_file_idx;
    if (fm->opened_info_len == fm->opened_info_cap) {
        mycc_grow_alloc((void**)&fm->opened_info,
                        &fm->opened_info_cap,
                        sizeof *fm->opened_info);
    }
    // The file that is currently open needs its file_loc stored for when we get
    // back to it
    fm->opened_info[fm->opened_info_len - 1].file_loc = s->line_info.curr_loc
                                                            .file_loc;
    fm->opened_info[fm->opened_info_len] = (struct opened_file_info){
        .idx = idx,
        .pos = -1,
        .file_loc = (struct file_loc){1, 1}};
    ++fm->opened_info_len;

    fm->files[fm->current_file_idx] = file;
    return true;
}

void preproc_state_close_file(struct preproc_state* s) {
    struct file_manager* fm = &s->file_manager;
    fclose(fm->files[fm->current_file_idx]);
    --fm->opened_info_len;
    if (fm->current_file_idx == 0) {
        const struct opened_file_info*
            info = &fm->opened_info[fm->opened_info_len - 1];
        const char* filename = str_get_data(&s->file_info.paths[info->idx]);
        FILE* file = fopen(filename, "r");
        int res = fseek(file, info->pos, SEEK_SET);
        assert(res == 0);
        UNUSED(res);

        fm->files[fm->current_file_idx] = file;
        s->line_info.next = NULL;
        s->line_info.curr_loc = (struct source_loc){info->idx, info->file_loc};
    } else {
        --fm->current_file_idx;
    }
}

const struct preproc_macro* find_preproc_macro(
    const struct preproc_state* state,
    const struct str* spelling) {
    return string_hash_map_get(&state->_macro_map, spelling);
}

void register_preproc_macro(struct preproc_state* state,
                            const struct str* spelling,
                            const struct preproc_macro* macro) {
    bool overwritten = string_hash_map_insert_overwrite(&state->_macro_map,
                                                        spelling,
                                                        macro);
    (void)overwritten; // TODO: warning if redefined
}

void remove_preproc_macro(struct preproc_state* state,
                          const struct str* spelling) {
    string_hash_map_remove(&state->_macro_map, spelling);
}

void push_preproc_cond(struct preproc_state* state,
                       struct source_loc loc,
                       bool was_true) {
    if (state->conds_len == state->conds_cap) {
        mycc_grow_alloc((void**)&state->conds,
                        &state->conds_cap,
                        sizeof *state->conds);
    }

    struct preproc_cond c = {
        .had_true_branch = was_true,
        .had_else = false,
        .loc = loc,
    };
    state->conds[state->conds_len] = c;
    ++state->conds_len;
}

void pop_preproc_cond(struct preproc_state* state) {
    --state->conds_len;
}

struct preproc_cond* peek_preproc_cond(struct preproc_state* state) {
    return &state->conds[state->conds_len - 1];
}

void free_token_arr(struct token_arr* arr) {
    for (size_t i = 0; i < arr->len; ++i) {
        free_str(&arr->tokens[i].spelling);
    }
    mycc_free(arr->tokens);
}

static void free_line_info(struct line_info* info) {
    free_str(&info->line);
}

static void free_file_manager(struct file_manager* fm) {
    if (fm->opened_info == NULL) {
        return;
    }
    mycc_free(fm->opened_info);
    for (size_t i = 0; i <= fm->current_file_idx; ++i) {
        fclose(fm->files[i]);
    }
}

void free_preproc_state(struct preproc_state* state) {
    free_token_arr(&state->res);
    free_line_info(&state->line_info);
    free_file_manager(&state->file_manager);
    mycc_free(state->conds);
    free_string_hash_map(&state->_macro_map);
    free_file_info(&state->file_info);
}

