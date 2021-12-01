#ifndef ERROR_H
#define ERROR_H

#include "token.h"

typedef enum {
    ERR_NONE,
    ERR_TOKENIZER,
    ERR_PARSER
} ErrorType;

/**
 * @brief Get the last error that was set
 * 
 * @return ErrorType The last error that was set using set_error() or set_error_file()
 */
ErrorType get_last_error();

/**
 * @brief Clears the global error state, so get_last_error() returns ERR_NONE
 * 
 */
void clear_last_error();

/**
 * @brief Get the message set by the last error. An error must have been set
 * 
 * @return const char* The error set by the last call to set_error() or set_error_file()
 */
const char* get_error_string();

/**
 * @brief Set the global error state
 * 
 * @param type The error type to set
 * @param format A format string, analogous to printf()
 * @param ... The strings format arguments
 */
void set_error(ErrorType type, const char* format, ...);

/**
 * @brief Sets the global error state, specifying the location of the error in the file
 * 
 * @param type The error type to set
 * @param filename Filename of the file that was trying to be compiled
 * @param loc SourceLocation in the given file
 * @param format A format string, analogous to printf()
 * @param ... The strings format arguments
 */
void set_error_file(ErrorType type, const char* filename, SourceLocation loc, const char* format, ...);

/**
 * @brief Appends to the already existing error message. The error must have already been set
 * 
 * @param format A format string, analogous to printf()
 * @param ... The strings format arguments
 */
void append_error_msg(const char* format, ...);

#endif

