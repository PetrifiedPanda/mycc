#include "frontend/preproc/PreprocState.h"

#include <string.h>
#include <ctype.h>

#include "util/mem.h"
#include "util/paths.h"

#include "frontend/preproc/PreprocMacro.h"

typedef struct OpenedFileInfo {
    size_t pos;
    char* data;
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

static char* read_entire_file(File f) {
    assert(File_valid(f));
    if (!File_seek(f, 0, FILE_SEEK_END)) {
        return NULL;
    }
    const long size = File_tell(f);
    if (!File_seek(f, 0, FILE_SEEK_START)) {
        return NULL;
    }

    char* data = mycc_alloc(size + 1);
    const size_t read = File_read(data, 1, size, f);
    if (read != (size_t)size) {
        return NULL;
    }
    data[size] = '\0';
    File_close(f);
    return data;
}

static FileData create_file_data(CStr start_file, PreprocErr* err) {
    StrBuf file_name = StrBuf_create(CStr_as_str(start_file));

    File file = File_open(start_file, FILE_READ | FILE_BINARY);
    if (!File_valid(file)) {
        PreprocErr_set_file_err(err,
                                &file_name,
                                (SourceLoc){UINT32_MAX, {0, 0}});
        return (FileData){0};
    }
    FileManager fm = {
        .opened_info_len = 1,
        .opened_info_cap = 1,
        .opened_info = mycc_alloc(sizeof *fm.opened_info),
        .prefixes_len = 1,
        .prefixes_cap = 1,
        .prefixes = mycc_alloc(sizeof *fm.prefixes),
    };
    char* data = read_entire_file(file);
    if (!data) {
        // TODO: error
        return (FileData){0};
    }
    fm.opened_info[0] = (OpenedFileInfo){
        .pos = 0,
        .data = data,
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

static PreprocMacroMap PreprocMacroMap_create(void) {
    return (PreprocMacroMap){0};
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
        .toks = PreprocTokenArr_create_empty(),
        .vals = PreprocTokenValList_create(),
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
        ._macro_map = PreprocMacroMap_create(),
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
        .toks = PreprocTokenArr_create_empty(),
        .vals = PreprocTokenValList_create(),
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
                .opened_info = NULL,
                .opened_info_len = 0,
                .opened_info_cap = 0,
            },
        .conds_len = 0,
        .conds_cap = 0,
        .conds = NULL,
        .err = err,
        ._macro_map = PreprocMacroMap_create(),
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

static bool current_file_over(const PreprocState* state) {
    const OpenedFileInfo* curr = &state->file_manager.opened_info[state->file_manager.opened_info_len - 1];
    return (state->line_info.next.data == NULL || *state->line_info.next.data == '\0') && (state->file_manager.opened_info == NULL || curr->data[curr->pos] == '\0');
}

static bool is_start_file(const PreprocState* state) {
    return state->file_manager.opened_info_len <= 1;
}

static Str read_next_line(OpenedFileInfo* info, StrBuf* line) {
    const char* data = info->data + info->pos;
    while (*data != '\n' && *data != '\r' && *data != '\0') {
        StrBuf_push_back(line, *data);
        ++data;
    }
    if (*data == '\n') {
        ++data;
    }
    if (*data == '\r') {
        ++data;
        // Handle windows line ending
        if (*data == '\n') {
            ++data;
        }
    }
    const size_t offset = data - info->data;
    info->pos = offset;
    return StrBuf_as_str(line);
}

static void preproc_state_close_file(PreprocState* s);

void PreprocState_read_line(PreprocState* state) {
    assert(state);
    StrBuf_clear(&state->line_info.line);
    while (current_file_over(state) && !is_start_file(state)) {
        preproc_state_close_file(state);
    }
    StrBuf* line = &state->line_info.line;
    OpenedFileInfo* opened_info = &state->file_manager.opened_info[state->file_manager.opened_info_len - 1];
    state->line_info.next = read_next_line(opened_info, line);
    while (is_escaped_newline(state->line_info.next)) {
        StrBuf_push_back(line, '\n');
        state->line_info.next = read_next_line(opened_info, line);
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
    File file = File_open(StrBuf_c_str(&full_path), FILE_READ | FILE_BINARY);
    if (!File_valid(file)) {
        errno = 0;
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

            file = File_open(StrBuf_c_str(&full_path), FILE_READ | FILE_BINARY);
            if (File_valid(file)) {
                StrBuf_shrink_to_fit(&full_path);
                break;
            }
            StrBuf_clear(&full_path);
        }

        if (!File_valid(file)) {
            StrBuf_free(&full_path);
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

    FileOpenRes fp = resolve_path_and_open(s, filename, include_loc);
    if (!File_valid(fp.file)) {
        return false;
    }
    FileInfo_add(&s->file_info, &fp.path);
    const uint32_t idx = s->file_info.len - 1;
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
    char* data = read_entire_file(fp.file);
    if (!data) {
        // TODO: error
        return false;
    }
    fm->opened_info[fm->opened_info_len] = (OpenedFileInfo){
        .pos = 0,
        .data = data,
        .prefix_idx = fp.prefix_idx,
        .loc = new_loc,
    };
    ++fm->opened_info_len;

    return true;
}

static void preproc_state_close_file(PreprocState* s) {
    FileManager* fm = &s->file_manager;
    const OpenedFileInfo* last_file = &fm->opened_info[fm->opened_info_len - 1];
    mycc_free(last_file->data);
    --fm->opened_info_len;
    const OpenedFileInfo* info = &fm->opened_info[fm->opened_info_len - 1];
    s->line_info.next = Str_null();
    s->line_info.curr_loc = info->loc;
}

static bool PreprocMacro_is_valid(const PreprocMacro* macro) {
    // A non-function macro cannot be variadic, so use this as invalid state
    return !(!macro->is_func_macro && macro->is_variadic);
}

static const PreprocMacro* PreprocMacroMap_find(const PreprocMacroMap* map,
                                                uint32_t idx) {
    if (idx < map->_cap) {
        const PreprocMacro* macro = &map->_macros[idx];
        if (PreprocMacro_is_valid(macro)) {
            return macro;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

const PreprocMacro* find_preproc_macro(const PreprocState* state,
                                       uint32_t identifier_idx) {
    return PreprocMacroMap_find(&state->_macro_map, identifier_idx);
}

static PreprocMacro PreprocMacro_create_invalid(void) {
    return (PreprocMacro){
        .is_func_macro = false,
        .is_variadic = true,
        .num_args = 0,
        .expansion_len = 0,
        .kinds = NULL,
        .vals = NULL,
    };
}

static bool PreprocMacroMap_register(PreprocMacroMap* map,
                              uint32_t idx,
                              const PreprocMacro* macro) {
    assert(PreprocMacro_is_valid(macro));
    if (idx >= map->_cap) {
        const uint32_t new_cap = idx + 1;
        map->_macros = mycc_realloc(map->_macros, sizeof *map->_macros * new_cap);
        for (uint32_t i = map->_cap; i < new_cap; ++i) {
            map->_macros[i] = PreprocMacro_create_invalid();
        }
        map->_cap = new_cap;
    }
    // If this overwrites a macro, free the old one
    PreprocMacro* old = &map->_macros[idx];
    bool overwritten = false;
    if (PreprocMacro_is_valid(old)) {
        PreprocMacro_free(old);
        overwritten = true;
    }
    *old = *macro;
    return overwritten;
}

void PreprocState_register_macro(PreprocState* state,
                                 uint32_t identifier_idx,
                                 const PreprocMacro* macro) {
    bool overwritten = PreprocMacroMap_register(&state->_macro_map,
                                                identifier_idx,
                                                macro);
    (void)overwritten; // TODO: warning if redefined
}

static void PreprocMacroMap_remove(PreprocMacroMap* map, uint32_t idx) {
    // It is possible to undef macros that are not defined
    if (idx >= map->_cap) {
        return;
    }
    PreprocMacro* macro = &map->_macros[idx];
    if (PreprocMacro_is_valid(macro)) {
        PreprocMacro_free(macro);
        *macro = PreprocMacro_create_invalid();
    }
}

void PreprocState_remove_macro(PreprocState* state, uint32_t identifier_idx) {
    PreprocMacroMap_remove(&state->_macro_map, identifier_idx);
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
    for (uint32_t i = 0; i < fm->opened_info_len; ++i) {
        mycc_free(fm->opened_info[i].data);
    }
    mycc_free(fm->opened_info);
    for (uint32_t i = 0; i < fm->prefixes_len; ++i) {
        StrBuf_free(&fm->prefixes[i]);
    }
    mycc_free(fm->prefixes);
}

static void PreprocMacroMap_free(const PreprocMacroMap* map) {
    for (uint32_t i = 0; i < map->_cap; ++i) {
        PreprocMacro_free(&map->_macros[i]);
    }
    mycc_free(map->_macros);
}

void PreprocState_free(PreprocState* state) {
    PreprocTokenArr_free(&state->toks);
    PreprocTokenValList_free(&state->vals);

    LineInfo_free(&state->line_info);
    FileManager_free(&state->file_manager);
    mycc_free(state->conds);
    PreprocMacroMap_free(&state->_macro_map);
    FileInfo_free(&state->file_info);
}

