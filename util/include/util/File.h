#ifndef MYCC_UTIL_FILE_H
#define MYCC_UTIL_FILE_H

#include <stdio.h>
#include <stdarg.h>

#include "Str.h"
#include "StrBuf.h"

typedef struct {
    FILE* _file;
} File;

typedef enum {
    FILE_WRITE = 1 << 0,
    FILE_READ = 1 << 1,
    FILE_APPEND = 1 << 2,
    FILE_BINARY = 1 << 3,
} OpenMode;

File File_open(CStr filename, OpenMode mode);

bool File_valid(File f);

bool File_close(File f);

bool File_flush(File f);

size_t File_read(void* res, size_t elem_bytes, size_t len, File f);
size_t File_write(const void* to_write, size_t elem_bytes, size_t len, File f);

typedef struct {
    bool valid;
    char res;
} FileGetcRes;

FileGetcRes File_getc(File f);

bool File_putc(char c, File f);

// TODO: overload for string literals?
bool File_put_str_val(Str str, File f);

#define File_put_str(str, f) File_put_str_val(STR_LIT(str), f)

bool File_ungetc(char c, File f);

// TODO: errors
void File_printf_impl(File f, Str format, ...);
void File_printf_varargs_impl(File f, Str format, va_list args);

#define mycc_printf(format, ...) File_printf_impl(mycc_stdout(), STR_LIT(format), __VA_ARGS__)
#define File_printf(f, format, ...) File_printf_impl(f, STR_LIT(format), __VA_ARGS__)
#define File_printf_varargs(f, format, args) File_printf_varargs_impl(f, STR_LIT(format), args)

long File_tell(File f);

typedef enum {
    FILE_SEEK_START,
    FILE_SEEK_CURR,
    FILE_SEEK_END,
} FileSeekOrigin;

// TODO: Origin with enum?
bool File_seek(File f, long offset, FileSeekOrigin origin);

Str File_read_line(File file,
                   StrBuf* str,
                   uint32_t* res_len,
                   char* static_buf,
                   uint32_t static_buf_len);

File mycc_stdout(void);
File mycc_stderr(void);

#endif

