#include "frontend/preproc/PreprocState.h"

#include <string.h>
#include <ctype.h>

#include "util/mem.h"
#include "util/file.h"
#include "util/macro_util.h"

#include "frontend/preproc/PreprocMacro.h"

typedef struct OpenedFileInfo {
    long pos;
    size_t prefix_idx;
    SourceLoc loc;
} OpenedFileInfo;

typedef struct {
    bool is_valid;
    FileManager fm;
    FileInfo fi;
} FileData;

static bool is_file_sep(char c) {
    switch (c) {
        case '/':
#ifdef _WIN32
        case '\\':
#endif
            return true;
        default:
            return false;
    }
}

static size_t get_last_file_sep(size_t len, const char* path) {
    const char* it = path + len - 1;
    const char* limit = path - 1;
    while (it != limit && !is_file_sep(*it)) {
        --it;
    }
    if (it == limit) {
        return (size_t)-1;
    } else {
        return it - path;
    }
}

static Str get_path_prefix(size_t len, const char* path) {
    size_t sep_idx = get_last_file_sep(len, path);
    if (sep_idx == (size_t)-1) {
        return Str_create_empty();
    } else {
        return Str_create(sep_idx + 1, path);
    }
}

static FileData create_file_data(const char* start_file, PreprocErr* err) {
    Str file_name = Str_create(strlen(start_file), start_file);

    FILE* file = fopen(start_file, "r");
    if (!file) {
        PreprocErr_set_file_err(err, &file_name, (SourceLoc){(size_t)-1, {0, 0}});
        return (FileData){0};
    }
    FileManager fm = {
        .files = {[0] = file},
        .opened_info_indices = {[0] = 0},
        .current_file_idx = 0,
        .opened_info_len = 1,
        .opened_info_cap = 1,
        .opened_info = mycc_alloc(sizeof *fm.opened_info),
        .prefixes_len = 1,
        .prefixes_cap = 1,
        .prefixes = mycc_alloc(sizeof *fm.prefixes),
    };
    fm.opened_info[0] = (OpenedFileInfo){
        .pos = -1,
        .prefix_idx = 0,
        .loc = {0, {0, 0}},
    };
    fm.prefixes[0] = get_path_prefix(strlen(start_file), start_file);
    FileInfo fi = FileInfo_create(&file_name);

    return (FileData){
        .is_valid = true,
        .fm = fm,
        .fi = fi,
    };
}

PreprocState PreprocState_create(const char* start_file, PreprocErr* err) {
    FileData fd = create_file_data(start_file, err);
    if (!fd.is_valid) {
        PreprocState res = {0};
        res.file_info = fd.fi;
        return res;
    }
    return (PreprocState){
        .res =
            {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            },
        .line_info =
            {
                .line = Str_create_empty(),
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
        ._macro_map = StringMap_create(sizeof(struct PreprocMacro),
                                        100,
                                        true,
                                        (void (*)(void*))PreprocMacro_free),
        .file_info = fd.fi,
    };
}

PreprocState PreprocState_create_string(const char* code,
                                         const char* filename,
                                         PreprocErr* err) {
    Str filename_str = Str_create(strlen(filename), filename);
    return (PreprocState){
        .res =
            {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            },
        .line_info =
            {
                .line = Str_create_empty(),
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
        ._macro_map = StringMap_create(sizeof(struct PreprocMacro),
                                        100,
                                        true,
                                        (void (*)(void*))PreprocMacro_free),
        .file_info = FileInfo_create(&filename_str),
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

static FILE* get_current_file(const PreprocState* state) {
    const FileManager* fm = &state->file_manager;
    return fm->files[fm->current_file_idx];
}

static bool current_file_over(const PreprocState* state) {
    FILE* file = get_current_file(state);
    bool file_is_over;
    if (file != NULL) {
        // TODO: there might be a better way to do this
        int next_char = getc(file);
        if (next_char == EOF) {
            file_is_over = true;
        } else {
            file_is_over = false;
            ungetc(next_char, file);
        }
    } else {
        file_is_over = true;
    }
    return (state->line_info.next == NULL || *state->line_info.next == '\0')
           && file_is_over;
}

static bool is_start_file(const PreprocState* state) {
    return state->file_manager.opened_info_len <= 1;
}

static void preproc_state_close_file(PreprocState* s);

void PreprocState_read_line(PreprocState* state) {
    assert(state);
    Str_clear(&state->line_info.line);
    size_t len = 0;
    enum {
        STATIC_BUF_LEN = ARR_LEN(state->line_info.static_buf),
    };
    while (current_file_over(state) && !is_start_file(state)) {
        preproc_state_close_file(state);
    }
    char* static_buf = state->line_info.static_buf;
    Str* line = &state->line_info.line;
    FILE* file = get_current_file(state);
    state->line_info.next = file_read_line(file,
                                           line,
                                           &len,
                                           static_buf,
                                           STATIC_BUF_LEN);
    while (is_escaped_newline(state->line_info.next, len)) {
        if (state->line_info.next == Str_get_data(line)) {
            Str_push_back(line, '\n');
        } else if (len < STATIC_BUF_LEN - 1) {
            static_buf[len] = '\n';
            static_buf[len + 1] = '\0';
        } else {
            assert(Str_len(line) == 0);
            Str_reserve(line, len + 1);
            Str_append_c_str(line, len, static_buf);
            Str_push_back(line, '\n');
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

bool PreprocState_over(const PreprocState* state) {
    return current_file_over(state) && is_start_file(state);
}

typedef struct {
    FILE* file;
    Str path;
    size_t prefix_idx;
} FileOpenRes;

static size_t get_current_prefix_idx(const FileManager* fm) {
    return fm->opened_info[fm->opened_info_len - 1].prefix_idx;
}

static void add_prefix(FileManager* fm, const Str* prefix) {
    if (fm->prefixes_len == fm->prefixes_cap) {
        mycc_grow_alloc((void**)&fm->prefixes,
                        &fm->prefixes_cap,
                        sizeof *fm->prefixes);
    }

    fm->prefixes[fm->prefixes_len] = *prefix;
    ++fm->prefixes_len;
}

// TODO: maybe use filename instead of creating a new string
static FileOpenRes resolve_path_and_open(PreprocState* s,
                                         const Str* filename,
                                         SourceLoc include_loc) {
    const size_t filename_len = Str_len(filename);
    const char* filename_data = Str_get_data(filename);
    const size_t sep_idx = get_last_file_sep(filename_len, filename_data);

    const size_t current_prefix_idx = get_current_prefix_idx(&s->file_manager);
    const Str* prefix = &s->file_manager.prefixes[current_prefix_idx];

    const size_t prefix_len = Str_len(prefix);
    const char* prefix_data = Str_get_data(prefix);
    Str full_path = Str_concat(prefix_len,
                               prefix_data,
                               filename_len,
                               filename_data);
    FILE* file = fopen(Str_get_data(&full_path), "r");
    if (!file) {
        // TODO: check include dirs (and system dirs)
        PreprocErr_set_file_err(s->err, filename, include_loc);
        return (FileOpenRes){0};
    }

    size_t prefix_idx;
    if (sep_idx != (size_t)-1) {
        Str new_prefix = Str_concat(prefix_len,
                                    prefix_data,
                                    sep_idx + 1,
                                    filename_data);
        add_prefix(&s->file_manager, &new_prefix);
        prefix_idx = s->file_manager.prefixes_len - 1;
    } else {
        prefix_idx = current_prefix_idx;
    }
    Str_free(filename);
    return (FileOpenRes){
        file,
        full_path,
        prefix_idx,
    };
}

bool PreprocState_open_file(PreprocState* s,
                             const Str* filename,
                             SourceLoc include_loc) {
    FileManager* fm = &s->file_manager;
    if (fm->current_file_idx == FOPEN_MAX - 1) {
        long pos = ftell(fm->files[0]);
        fclose(fm->files[0]);
        fm->opened_info[fm->opened_info_indices[0]].pos = pos;
        memmove(fm->files, fm->files + 1, sizeof fm->files - sizeof(FILE*));
        memmove(fm->opened_info_indices,
                fm->opened_info_indices + 1,
                sizeof fm->opened_info_indices
                    - sizeof *fm->opened_info_indices);
        --fm->current_file_idx;
    }

    FileOpenRes fp = resolve_path_and_open(s, filename, include_loc);
    if (!fp.file) {
        return false;
    }
    FileInfo_add(&s->file_info, &fp.path);
    const size_t idx = s->file_info.len - 1;
    ++fm->current_file_idx;
    if (fm->opened_info_len == fm->opened_info_cap) {
        mycc_grow_alloc((void**)&fm->opened_info,
                        &fm->opened_info_cap,
                        sizeof *fm->opened_info);
    }
    // The file that is currently open needs its file_loc stored for when we get
    // back to it
    assert(s->line_info.curr_loc.file_idx
           == fm->opened_info[fm->opened_info_len - 1].loc.file_idx);
    fm->opened_info[fm->opened_info_len - 1].loc = s->line_info.curr_loc;
    const SourceLoc new_loc = {
        .file_idx = idx,
        .file_loc = {0, 1},
    };

    s->line_info.curr_loc = new_loc;
    fm->opened_info[fm->opened_info_len] = (OpenedFileInfo){
        .pos = -1,
        .prefix_idx = fp.prefix_idx,
        .loc = new_loc,
    };
    ++fm->opened_info_len;

    fm->files[fm->current_file_idx] = fp.file;
    fm->opened_info_indices[fm->current_file_idx] = fm->opened_info_len - 1;
    return true;
}

static void preproc_state_close_file(PreprocState* s) {
    FileManager* fm = &s->file_manager;
    fclose(fm->files[fm->current_file_idx]);
    --fm->opened_info_len;
    const OpenedFileInfo* info = &fm->opened_info[fm->opened_info_len - 1];
    if (fm->current_file_idx == 0) {
        const char* filename = Str_get_data(
            &s->file_info.paths[info->loc.file_idx]);
        FILE* file = fopen(filename, "r");
        int res = fseek(file, info->pos, SEEK_SET);
        assert(res == 0);
        UNUSED(res);

        fm->files[fm->current_file_idx] = file;
    } else {
        --fm->current_file_idx;
    }
    s->line_info.next = NULL;
    s->line_info.curr_loc = info->loc;
}

const PreprocMacro* find_preproc_macro(const PreprocState* state, const Str* spelling) {
    return StringMap_get(&state->_macro_map, spelling);
}

void register_preproc_macro(PreprocState* state,
                            const Str* spelling,
                            const PreprocMacro* macro) {
    bool overwritten = StringMap_insert_overwrite(&state->_macro_map,
                                                   spelling,
                                                   macro);
    (void)overwritten; // TODO: warning if redefined
}

void remove_preproc_macro(PreprocState* state, const Str* spelling) {
    StringMap_remove(&state->_macro_map, spelling);
}

void push_preproc_cond(PreprocState* state,
                       SourceLoc loc,
                       bool was_true) {
    if (state->conds_len == state->conds_cap) {
        mycc_grow_alloc((void**)&state->conds,
                        &state->conds_cap,
                        sizeof *state->conds);
    }

    PreprocCond c = {
        .had_true_branch = was_true,
        .had_else = false,
        .loc = loc,
    };
    state->conds[state->conds_len] = c;
    ++state->conds_len;
}

void pop_preproc_cond(PreprocState* state) {
    --state->conds_len;
}

PreprocCond* peek_preproc_cond(PreprocState* state) {
    return &state->conds[state->conds_len - 1];
}

void TokenArr_free(TokenArr* arr) {
    for (size_t i = 0; i < arr->len; ++i) {
        Str_free(&arr->tokens[i].spelling);
    }
    mycc_free(arr->tokens);
}

static void free_line_info(LineInfo* info) {
    Str_free(&info->line);
}

static void free_file_manager(FileManager* fm) {
    if (fm->opened_info == NULL) {
        return;
    }
    mycc_free(fm->opened_info);
    for (size_t i = 0; i <= fm->current_file_idx; ++i) {
        fclose(fm->files[i]);
    }
    for (size_t i = 0; i < fm->prefixes_len; ++i) {
        Str_free(&fm->prefixes[i]);
    }
    mycc_free(fm->prefixes);
}

void PreprocState_free(PreprocState* state) {
    TokenArr_free(&state->res);
    free_line_info(&state->line_info);
    free_file_manager(&state->file_manager);
    mycc_free(state->conds);
    StringMap_free(&state->_macro_map);
    FileInfo_free(&state->file_info);
}

