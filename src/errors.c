/**
 * @file errors.c
 * @brief Handle errors gracefully, and exit with the correct failure code
 */

#include "errors.h"

#include <stdlib.h>
#include <string.h>

// Handle any exit (successful or failure)
// O(1) time
_Noreturn void handleExit(int errorNum, const char* file, size_t row, size_t col, int savedErrno, const char* extra) {
    errno = savedErrno;

    if (errorNum != SUCCESS && errorNum != USAGEREQ) {
        fprintf(S_E, "Error.%04d: ", errorNum);
    }
    switch (errorNum) {
        case SUCCESS:
            fprintf(S_O, "Chasm compilation completed successfully (exited with code 0)\n");
            exit(COMPILED_SUCCESSFULLY);
            
        case UNTERMINATED_BLOCK_COMMENT_ERROR:
            fprintf(S_E, "unterminated block comment at %s:%zu:%zu\n", file, row, col);
            exit(EXIT_COMPILER_ERROR);
        case BAD_FILE_DESCRIPTOR_ERROR:
            fprintf(S_E, "`%s` has a bad file descriptor\n", file);
            exit(EXIT_OS_ERROR);
        case FILE_TOO_LONG_ERROR:
            fprintf(S_E, "`%s` was too long to read\n", file);
            exit(EXIT_OS_ERROR);
        case POSITIONING_FAILED_ERROR:
            fprintf(S_E, "`%s` does not support positioning or seeking\n", file);
            exit(EXIT_OS_ERROR);
        case INVALID_FTELL_ARG_ERROR:
            fprintf(S_E, "Invalid argument was passed to `ftell`\n");
            exit(EXIT_OS_ERROR);
        // No Error.0005 - at the end for default to fall through to it
        case NULL_FILE_LEXER_ERROR:
            fprintf(S_E, "Lexer was given a non-existent file\n");
            exit(EXIT_OS_ERROR);
        case FILE_NOT_FOUND_ERROR:
            fprintf(S_E, "`%s` was not found at the given location\n", file);
            exit(EXIT_OS_ERROR);
        case FILE_PERMISSIONS_DENIED_ERROR:
            fprintf(S_E, "Permissions were denied for `%s`\n", file);
            exit(EXIT_OS_ERROR);
        case CALL_INTERRUPTED_ERROR:
            fprintf(S_E, "Compilation was interrupted before it completed\n");
            exit(EXIT_OS_ERROR);
        case MAX_OPEN_FILES_ERROR:
            fprintf(S_E, "Too many files are currently open to compile the given file\n");
            exit(EXIT_OS_ERROR);
        case DIRECTORY_TOO_LONG_ERROR:
            fprintf(S_E, "The directory name given is too long: `%s`\n", file);
            exit(EXIT_OS_ERROR);
        case READ_ONLY_FILESYSTEM_ERROR:
            fprintf(S_E, "The filesystem is read-only\n");
            exit(EXIT_OS_ERROR);
        case IO_ERROR:
            fprintf(S_E, "An IO error occurred: %s\n", strerror(errno));
            exit(EXIT_OS_ERROR);
        case MEM_ERROR:
            fprintf(S_E, "The system ran out of memory\n");
            exit(EXIT_MEMORY_ERROR);
        case UNAVAIL_RESOURCE_ERROR:
            fprintf(S_E, "The resource is unavailable\n");
            exit(EXIT_OS_ERROR);

        case USAGEREQ:
            fprintf(S_O, "Usage: cmc <file>\n");
            exit(EXIT_NO_ERROR);
        case UNKNOWN_ERROR:
            fprintf(S_E, "An unknown error occurred while compiling `%s` (%s)\nOUTPUT: %d: %s\n", file, extra, errno, strerror(errno));
            exit(EXIT_GENERAL_ERROR);
        default:
            fprintf(S_E, "An undefined error (#%d) occurred while compiling `%s`\nOUTPUT: %d: %s\n", errorNum, file, errno, strerror(errno));
            exit(EXIT_GENERAL_ERROR);
    }
}