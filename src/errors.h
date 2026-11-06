/**
 * @file errors.h
 * @brief Header file with error code definitions for errors.c
 */

#ifndef CHASM_ERRORS_H
#define CHASM_ERRORS_H

#include <stdio.h>
#include <errno.h>

// Success Numbers
#define SUCCESS -1
#define USAGEREQ -2

// Error Numbers
#define UNTERMINATED_BLOCK_COMMENT_ERROR 0
#define BAD_FILE_DESCRIPTOR_ERROR 1
#define FILE_TOO_LONG_ERROR 2
#define POSITIONING_FAILED_ERROR 3
#define INVALID_FTELL_ARG_ERROR 4
#define UNKNOWN_ERROR 5
#define NULL_FILE_LEXER_ERROR 6
#define FILE_NOT_FOUND_ERROR 7
#define FILE_PERMISSIONS_DENIED_ERROR 8
#define CALL_INTERRUPTED_ERROR 9
#define MAX_OPEN_FILES_ERROR 10
#define DIRECTORY_TOO_LONG_ERROR 11
#define READ_ONLY_FILESYSTEM_ERROR 12
#define IO_ERROR 13
#define MEM_ERROR 14
#define UNAVAIL_RESOURCE_ERROR 15
#define UNTERMINATED_STRING_ERROR 16
#define EMBEDDED_NUL_ERROR 18

// Misc. Definitions for QOL
#define S_E stderr
#define S_O stdout
#define ERRNO_RESET errno = 0
#define SAVE_ERRNO int serrno = errno
#define LOAD_ERRNO serrno
#define NO_ROWCOL_GIVEN 0, 0
#define EMPTY_STR ""
#define COMPILED_SUCCESSFULLY 0
#define EXIT_GENERAL_ERROR 1
#define EXIT_NO_ERROR 3
#define EXIT_COMPILER_ERROR 4
#define EXIT_MEMORY_ERROR 5
#define EXIT_OS_ERROR 6

_Noreturn void handleExit(int errorNum, const char* file, size_t row, size_t col, int savedErrno, const char* extra);

#endif