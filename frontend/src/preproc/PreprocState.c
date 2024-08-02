#include "frontend/preproc/PreprocState.h"

#include <string.h>
#include <ctype.h>

#include "util/mem.h"
#include "util/paths.h"
#include "util/macro_util.h"

#include "frontend/preproc/PreprocMacro.h"

typedef struct OpenedFileInfo {
    long pos;
    uint32_t prefix_idx;
    SourceLoc loc;
} OpenedFileInfo;

typedef struct {
    bool is_valid;
    FileManager fm;
    FileInfo fi;
} FileData;

static StrBuf get_path_prefix(Str path) {
    uint32_t sep_idx = get_last_file_sep(path);
    if (sep_idx == UINT32_MAX) {
        return StrBuf_create_empty();
    } else {
        return StrBuf_create(Str_substr(path, 0, sep_idx + 1));
    }
}

static FileData create_file_data(CStr start_file, PreprocErr* err) {
    StrBuf file_name = StrBuf_create(CStr_as_str(start_file));

    File file = File_open(start_file, FILE_READ);
    if (!File_valid(file)) {
        PreprocErr_set_file_err(err,
                                &file_name,
                                (SourceLoc){UINT32_MAX, {0, 0}});
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
    fm.prefixes[0] = get_path_prefix(CStr_as_str(start_file));
    FileInfo fi = FileInfo_create(&file_name);

    return (FileData){
        .is_valid = true,
        .fm = fm,
        .fi = fi,
    };
}

PreprocState PreprocState_create(CStr start_file,
                                 uint32_t num_include_dirs,
                                 const Str* include_dirs,
                                 PreprocErr* err) {
    FileData fd = create_file_data(start_file, err);
    if (!fd.is_valid) {
        PreprocState res = {0};
        res.file_info = fd.fi;
        return res;
    }
    return (PreprocState){
        .res = PreprocTokenArr_create_empty(),
        .line_info =
            {
                .line = StrBuf_create_empty(),
                .next = Str_null(),
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
        .num_include_dirs = num_include_dirs,
        .include_dirs = include_dirs,
        .file_info = fd.fi,
    };
}

PreprocState PreprocState_create_string(Str code,
                                        Str filename,
                                        uint32_t num_include_dirs,
                                        const Str* include_dirs,
                                        PreprocErr* err) {
    StrBuf filename_str = StrBuf_create(filename);
    return (PreprocState){
        .res = PreprocTokenArr_create_empty(),
        .line_info =
            {
                .line = StrBuf_create_empty(),
                .next = code,
                .curr_loc =
                    {
                        .file_idx = 0,
                        .file_loc = {1, 1},
                    },
            },
        .file_manager =
            {
                .files = {{0}},
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
        .num_include_dirs = num_include_dirs,
        .include_dirs = include_dirs,
        .file_info = FileInfo_create(&filename_str),
    };
}

static bool is_escaped_newline(Str line) {
    if (line.len == 0) {
        return false;
    }

    uint32_t i = line.len - 1;
    while (i != 0 && isspace(Str_at(line, i))) {
        --i;
    }
    return Str_at(line, i) == '\\';
}

static File get_current_file(const PreprocState* state) {
    const FileManager* fm = &state->file_manager;
    return fm->files[fm->current_file_idx];
}

static bool current_file_over(const PreprocState* state) {
    File file = get_current_file(state);
    bool file_is_over;
    if (File_valid(file)) {
        // TODO: there might be a better way to do this
        FileGetcRes next_char = File_getc(file);
        if (!next_char.valid) {
            file_is_over = true;
        } else {
            file_is_over = false;
            File_ungetc(next_char.res, file);
        }
    } else {
        file_is_over = true;
    }
    return (state->line_info.next.data == NULL
            || *state->line_info.next.data == '\0')
           && file_is_over;
}

static bool is_start_file(const PreprocState* state) {
    return state->file_manager.opened_info_len <= 1;
}

static void preproc_state_close_file(PreprocState* s);

void PreprocState_read_line(PreprocState* state) {
    assert(state);
    StrBuf_clear(&state->line_info.line);
    while (current_file_over(state) && !is_start_file(state)) {
        preproc_state_close_file(state);
    }
    StrBuf* line = &state->line_info.line;
    File file = get_current_file(state);
    state->line_info.next = File_read_line(file, line);
    while (is_escaped_newline(state->line_info.next)) {
        StrBuf_push_back(line, '\n');
        state->line_info.next = File_read_line(file, line);
    }
    state->line_info.curr_loc.file_loc.line += 1;
    state->line_info.curr_loc.file_loc.index = 1;
}

bool PreprocState_over(const PreprocState* state) {
    return current_file_over(state) && is_start_file(state);
}

typedef struct {
    File file;
    StrBuf path;
    uint32_t prefix_idx;
} FileOpenRes;

static uint32_t get_current_prefix_idx(const FileManager* fm) {
    return fm->opened_info[fm->opened_info_len - 1].prefix_idx;
}

static void add_prefix(FileManager* fm, const StrBuf* prefix) {
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
                                         const StrBuf* filename,
                                         const SourceLoc* include_loc) {
    const Str filename_str = StrBuf_as_str(filename);
    const uint32_t sep_idx = get_last_file_sep(filename_str);

    const uint32_t current_prefix_idx = get_current_prefix_idx(
        &s->file_manager);
    const StrBuf* prefix = &s->file_manager.prefixes[current_prefix_idx];

    const Str prefix_str = StrBuf_as_str(prefix);
    StrBuf full_path = StrBuf_concat(prefix_str, filename_str);
    File file = File_open(StrBuf_c_str(&full_path), FILE_READ);
    if (!File_valid(file)) {
        StrBuf_clear(&full_path);
        for (uint32_t i = 0; i < s->num_include_dirs; ++i) {
            const Str dir = s->include_dirs[i];

            uint32_t needed_cap = dir.len + StrBuf_len(filename);
            if (!is_file_sep(Str_at(dir, dir.len - 1))) {
                needed_cap += 1;
                StrBuf_reserve(&full_path, needed_cap);
                StrBuf_append(&full_path, dir);
                StrBuf_push_back(&full_path, '/');
            } else {
                StrBuf_reserve(&full_path, needed_cap);
                StrBuf_append(&full_path, dir);
            }
            StrBuf_append(&full_path, filename_str);

            file = File_open(StrBuf_c_str(&full_path), FILE_READ);
            if (File_valid(file)) {
                StrBuf_shrink_to_fit(&full_path);
                break;
            }
            StrBuf_clear(&full_path);
        }

        if (!File_valid(file)) {
            // TODO: check system dirs?
            PreprocErr_set_file_err(s->err, filename, *include_loc);
            return (FileOpenRes){0};
        }
    }

    uint32_t prefix_idx;
    if (sep_idx != UINT32_MAX) {
        // TODO: prefix_str may not be valid anymore
        StrBuf new_prefix = StrBuf_concat(
            prefix_str,
            Str_substr(filename_str, 0, sep_idx + 1));
        add_prefix(&s->file_manager, &new_prefix);
        prefix_idx = s->file_manager.prefixes_len - 1;
    } else {
        prefix_idx = current_prefix_idx;
    }
    StrBuf_free(filename);
    return (FileOpenRes){
        file,
        full_path,
        prefix_idx,
    };
}

bool PreprocState_open_file(PreprocState* s,
                            const StrBuf* filename,
                            const SourceLoc* include_loc) {
    FileManager* fm = &s->file_manager;
    if (fm->current_file_idx == FOPEN_MAX - 1) {
        long pos = File_tell(fm->files[0]);
        File_close(fm->files[0]);
        fm->opened_info[fm->opened_info_indices[0]].pos = pos;
        memmove(fm->files, fm->files + 1, sizeof fm->files - sizeof(File));
        memmove(fm->opened_info_indices,
                fm->opened_info_indices + 1,
                sizeof fm->opened_info_indices
                    - sizeof *fm->opened_info_indices);
        --fm->current_file_idx;
    }

    FileOpenRes fp = resolve_path_and_open(s, filename, include_loc);
    if (!File_valid(fp.file)) {
        return false;
    }
    FileInfo_add(&s->file_info, &fp.path);
    const uint32_t idx = s->file_info.len - 1;
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
    File_close(fm->files[fm->current_file_idx]);
    --fm->opened_info_len;
    const OpenedFileInfo* info = &fm->opened_info[fm->opened_info_len - 1];
    if (fm->current_file_idx == 0) {
        CStr filename = StrBuf_c_str(&s->file_info.paths[info->loc.file_idx]);
        File file = File_open(filename, FILE_READ);
        bool res = File_seek(file, info->pos, SEEK_SET);
        assert(res);
        UNUSED(res);

        fm->files[fm->current_file_idx] = file;
    } else {
        --fm->current_file_idx;
    }
    s->line_info.next = Str_null();
    s->line_info.curr_loc = info->loc;
}

const PreprocMacro* find_preproc_macro(const PreprocState* state,
                                       const StrBuf* spelling) {
    return StringMap_get(&state->_macro_map, StrBuf_as_str(spelling));
}

void PreprocState_register_macro(PreprocState* state,
                                 const StrBuf* spelling,
                                 const PreprocMacro* macro) {
    bool overwritten = StringMap_insert_overwrite(&state->_macro_map,
                                                  spelling,
                                                  macro);
    (void)overwritten; // TODO: warning if redefined
}

void PreprocState_remove_macro(PreprocState* state, const StrBuf* spelling) {
    StringMap_remove(&state->_macro_map, spelling);
}

void PreprocState_push_cond(PreprocState* state, SourceLoc loc, bool was_true) {
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

void PreprocState_pop_cond(PreprocState* state) {
    --state->conds_len;
}

PreprocCond* peek_preproc_cond(PreprocState* state) {
    assert(state->conds_len != 0);
    return &state->conds[state->conds_len - 1];
}

static void LineInfo_free(LineInfo* info) {
    StrBuf_free(&info->line);
}

static void FileManager_free(FileManager* fm) {
    if (fm->opened_info == NULL) {
        return;
    }
    mycc_free(fm->opened_info);
    for (uint32_t i = 0; i <= fm->current_file_idx; ++i) {
        File_close(fm->files[i]);
    }
    for (uint32_t i = 0; i < fm->prefixes_len; ++i) {
        StrBuf_free(&fm->prefixes[i]);
    }
    mycc_free(fm->prefixes);
}

void PreprocState_free(PreprocState* state) {
    PreprocTokenArr_free(&state->res);
    LineInfo_free(&state->line_info);
    FileManager_free(&state->file_manager);
    mycc_free(state->conds);
    StringMap_free(&state->_macro_map);
    FileInfo_free(&state->file_info);
}

