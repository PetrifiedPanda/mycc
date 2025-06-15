#include "util/File.h"

#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#include "util/macro_util.h"

enum {
    OPENMODE_MAX_LEN = 4,
};

// TODO: asserts for incompatible flags
static void get_mode_str(OpenMode mode, char buf[OPENMODE_MAX_LEN]) {
    uint32_t i = 0;
    if (mode & FILE_WRITE) {
        buf[i] = 'w';
        ++i;
    }

    if (mode & FILE_READ) {
        buf[i] = 'r';
        ++i;
    }

    if (mode & FILE_APPEND) {
        buf[i] = 'a';
        ++i;
    }

    if (mode & FILE_BINARY) {
        buf[i] = 'b';
        ++i;
    }
    assert(i <= OPENMODE_MAX_LEN);
}

File File_open(CStr filename, OpenMode mode) {
    char mode_str[OPENMODE_MAX_LEN] = {0};
    get_mode_str(mode, mode_str);

    return (File){
        ._file = fopen(filename.data, mode_str),
    };
}

bool File_valid(File f) {
    return f._file != NULL;
}

bool File_close(File f) {
    const int res = fclose(f._file);
    return res == 0;
}

bool File_flush(File f) {
    const int res = fflush(f._file);
    return res == 0;
}

size_t File_read(void* res, size_t elem_bytes, size_t len, File f) {
    return fread(res, elem_bytes, len, f._file);
}

size_t File_write(const void* to_write, size_t elem_bytes, size_t len, File f) {
    return fwrite(to_write, elem_bytes, len, f._file);
}

FileGetcRes File_getc(File f) {
    const int res = getc(f._file);
    if (res == EOF) {
        return (FileGetcRes){
            .valid = false,
        };
    }

    return (FileGetcRes){
        .valid = true,
        .res = (char)(unsigned char)res,
    };
}

bool File_putc(char c, File f) {
    const int input = c;
    const int res = putc(input, f._file);
    return res != EOF;
}

bool File_put_str_val(Str str, File f) {
    return fwrite(str.data, 1, str.len, f._file) == str.len;
}

Str get_format_str(Str bracket_start) {
    assert(Str_at(bracket_start, 0) == '{');
    assert(bracket_start.len > 1);
    for (uint32_t i = 1; i < bracket_start.len; ++i) {
        if (Str_at(bracket_start, i) == '}') {
            return Str_substr(bracket_start, 1, i);
        }
    }
    UNREACHABLE();
}

void File_printf_impl(File f, Str format, ...) {
    va_list list;
    va_start(list, format);
    File_printf_varargs_impl(f, format, list);
    va_end(list);
}

void File_printf_varargs_impl(File f, Str format, va_list args) {
    for (uint32_t i = 0; i < format.len; ++i) {
        const char curr = Str_at(format, i);
        if (curr == '{') {
            const Str format_str = get_format_str(Str_advance(format, i));
            if (Str_eq(format_str, STR_LIT("size_t"))) {
                size_t arg = va_arg(args, size_t);
                fprintf(f._file, "%zu", arg);
            } else if (Str_eq(format_str, STR_LIT("u64"))) {
                uint64_t arg = va_arg(args, uint64_t);
                fprintf(f._file, "%" PRIu64, arg);
            } else if (Str_eq(format_str, STR_LIT("i64"))) {
                int64_t arg = va_arg(args, int64_t);
                fprintf(f._file, "%" PRId64, arg);
            } else if (Str_eq(format_str, STR_LIT("u32"))) {
                uint32_t arg = va_arg(args, uint32_t);
                fprintf(f._file, "%" PRIu32, arg);
            } else if (Str_eq(format_str, STR_LIT("Str"))) {
                Str arg = va_arg(args, Str);
                fwrite(arg.data, 1, arg.len, f._file);
            } else if (Str_eq(format_str, STR_LIT("intmax"))) {
                intmax_t arg = va_arg(args, intmax_t);
                fprintf(f._file, "%" PRIiMAX, arg);
            } else if (Str_eq(format_str, STR_LIT("uintmax"))) {
                uintmax_t arg = va_arg(args, uintmax_t);
                fprintf(f._file, "%" PRIuMAX, arg);
            } else if (Str_eq(format_str, STR_LIT("uint"))) {
                unsigned arg = va_arg(args, unsigned);
                fprintf(f._file, "%u", arg);
            } else if (Str_eq(format_str, STR_LIT("int"))) {
                int arg = va_arg(args, int);
                fprintf(f._file, "%d", arg);
            } else if (Str_eq(format_str, STR_LIT("ptr"))) {
                void* arg = va_arg(args, void*);
                fprintf(f._file, "%p", arg);
            } else if (Str_eq(format_str, STR_LIT("char"))) {
                int arg = va_arg(args, int);
                fprintf(f._file, "%c", arg);
            } else if (Str_eq(format_str, STR_LIT("bool"))) {
                bool arg = va_arg(args, int);
                const Str to_print = arg ? STR_LIT("true") : STR_LIT("false");
                File_put_str_val(to_print, f);
            } else if (Str_eq(format_str, STR_LIT("c_str"))) {
                const char* arg = va_arg(args, const char*);
                fprintf(f._file, "%s", arg);
            } else if (Str_starts_with(format_str, STR_LIT("float"))) {
                double arg = va_arg(args, double);
                const Str float_str = STR_LIT("float");
                if (format_str.len == float_str.len) {
                    fprintf(f._file, "%f", arg);
                } else {
                    const Str suffix = Str_advance(format_str, float_str.len);
                    char float_format[100];
                    const char first = Str_at(suffix, 0);
                    if (first == '.') {
                        float_format[0] = '%';
                        assert(suffix.len < sizeof float_format - 3);
                        memcpy(float_format + 1, suffix.data, suffix.len);
                        float_format[suffix.len + 1] = 'f';
                        float_format[suffix.len + 2] = '\0';
                        fprintf(f._file, float_format, arg);
                    } else if (first == 'g') {
                        assert(suffix.len == 1);
                        fprintf(f._file, "%g", arg);
                    }
                }
            } else {
                UNREACHABLE();
            }
            i += format_str.len + 1;
        } else {
            putc(curr, f._file);
        }
    }
}

long File_tell(File f) {
    return ftell(f._file);
}

bool File_seek(File f, long offset, FileSeekOrigin origin) {
    const int origin_conv = (origin == FILE_SEEK_START)
                                ? SEEK_SET
                                : ((origin == FILE_SEEK_CURR) ? SEEK_CUR
                                                              : SEEK_END);
    const int res = fseek(f._file, offset, origin_conv);
    return res == 0;
}

#ifndef _WIN32

static void handle_win_line_ending(char newline_char, File file) {
    if (newline_char == '\r') {
        FileGetcRes next = File_getc(file);
        if (next.valid && next.res != '\n') {
            File_ungetc(next.res, file);
        }
    }
}

#endif

Str File_read_line(File file, StrBuf* res) {
    assert(res);

    FileGetcRes c;

    while ((c = File_getc(file)).valid && c.res != '\n' && c.res != '\r') {
        StrBuf_push_back(res, c.res);
    }
#ifndef _WIN32
    handle_win_line_ending(c.res, file);
#endif
    if (StrBuf_len(res) == 0 && !c.valid) {
        return Str_null();
    }
    return StrBuf_as_str(res);
}
