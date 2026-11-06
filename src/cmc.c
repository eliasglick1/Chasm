/**
 * @file cmc.c
 * @brief Main compiler for Chasm
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "lexer.h"
#include "errors.h"

#define FTELL_FAIL_VAL -1L
#define CF_SUCCESS 0

static void getFirstNUL(size_t* r, size_t* c, const char* buf, long fz) {
    const char* p = buf;
    *r = *c = 1;
    while (*p != '\0' && p - buf != fz) {
        if (*p == '\n') { (*r)++; *c = 1; }
        else { (*c)++; }
        p++;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        #define FILENAME argv[1]
        ERRNO_RESET;
        FILE* fp = fopen(FILENAME, "rb");
        if (!fp) {
            switch (errno) {
                case ENOENT:
                    handleExit(FILE_NOT_FOUND_ERROR, FILENAME, NO_ROWCOL_GIVEN, errno, EMPTY_STR);
                case EACCES:
                    handleExit(FILE_PERMISSIONS_DENIED_ERROR, FILENAME, NO_ROWCOL_GIVEN, errno, EMPTY_STR);
                case EINTR:
                    handleExit(CALL_INTERRUPTED_ERROR, EMPTY_STR, NO_ROWCOL_GIVEN, errno, EMPTY_STR);
                case ENFILE:
                    handleExit(MAX_OPEN_FILES_ERROR, EMPTY_STR, NO_ROWCOL_GIVEN, errno, EMPTY_STR);
                case ENAMETOOLONG:
                    handleExit(DIRECTORY_TOO_LONG_ERROR, FILENAME, NO_ROWCOL_GIVEN, errno, EMPTY_STR);
                default:
                    handleExit(UNKNOWN_ERROR, FILENAME, NO_ROWCOL_GIVEN, errno, "error occurred while reading file");
            }
        }

        ERRNO_RESET;
        if (fseek(fp, 0, SEEK_END) != CF_SUCCESS) {
            SAVE_ERRNO;
            fclose(fp);
            switch (LOAD_ERRNO) {
                case ESPIPE:
                    handleExit(POSITIONING_FAILED_ERROR, FILENAME, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                case EOVERFLOW:
                    handleExit(FILE_TOO_LONG_ERROR, FILENAME, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                case EBADF:
                    handleExit(BAD_FILE_DESCRIPTOR_ERROR, FILENAME, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                case EIO:
                    handleExit(IO_ERROR, EMPTY_STR, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                case EINTR:
                    handleExit(CALL_INTERRUPTED_ERROR, EMPTY_STR, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                default:
                    handleExit(UNKNOWN_ERROR, FILENAME, NO_ROWCOL_GIVEN, LOAD_ERRNO, "error occurred while getting file size");
            }
        }

        ERRNO_RESET;
        long fsize = ftell(fp);
        if (fsize == FTELL_FAIL_VAL) {
            SAVE_ERRNO;
            fclose(fp);
            switch (LOAD_ERRNO) {
                case EBADF:
                    handleExit(BAD_FILE_DESCRIPTOR_ERROR, FILENAME, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                case EOVERFLOW:
                    handleExit(FILE_TOO_LONG_ERROR, FILENAME, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                case ESPIPE:
                    handleExit(POSITIONING_FAILED_ERROR, FILENAME, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                case EINVAL:
                    handleExit(INVALID_FTELL_ARG_ERROR, FILENAME, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                default:
                    handleExit(UNKNOWN_ERROR, FILENAME, NO_ROWCOL_GIVEN, LOAD_ERRNO, "error occurred while getting file size");
            }
        }
        rewind(fp);

        ERRNO_RESET;
        // No safety on file size wrapping because ULLONG_MAX is too big for any normal filesystem
        char* buf = malloc((size_t)fsize + 1); 
        if (!buf) {
            SAVE_ERRNO;
            fclose(fp);
            switch (LOAD_ERRNO) {
                case ENOMEM:
                    handleExit(MEM_ERROR, EMPTY_STR, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                case EAGAIN:
                    handleExit(UNAVAIL_RESOURCE_ERROR, EMPTY_STR, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                default:
                    handleExit(UNKNOWN_ERROR, FILENAME, NO_ROWCOL_GIVEN, LOAD_ERRNO, "error occurred while allocating memory");
            }
        }

        ERRNO_RESET;
        // File size in bytes
        size_t bytesRead = fread(buf, 1, (size_t)fsize, fp);
        SAVE_ERRNO;
        fclose(fp);

        if (bytesRead != (size_t)fsize) {
            free(buf);
            switch (LOAD_ERRNO) {
                case EAGAIN:
                    handleExit(UNAVAIL_RESOURCE_ERROR, EMPTY_STR, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                case EBADF:
                    handleExit(BAD_FILE_DESCRIPTOR_ERROR, FILENAME, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                case EINTR:
                    handleExit(CALL_INTERRUPTED_ERROR, EMPTY_STR, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                case EIO:
                    handleExit(IO_ERROR, EMPTY_STR, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                case EOVERFLOW:
                    handleExit(FILE_TOO_LONG_ERROR, FILENAME, NO_ROWCOL_GIVEN, LOAD_ERRNO, EMPTY_STR);
                default:
                    handleExit(UNKNOWN_ERROR, FILENAME, NO_ROWCOL_GIVEN, LOAD_ERRNO, "error occurred while reading file into buffer");
            }
        }

        // Check for BOM
        const char* src;
        long fsrcSize;
        if (fsize >= 3 && memcmp(buf, "\xEF\xBB\xBF", 3) == 0) { src = buf + 3; fsrcSize = fsize - 3; }
        else { src = buf; fsrcSize = fsize; }
        
        if (memchr(src, '\0', (size_t)fsrcSize) != NULL) {
            size_t row, col;
            getFirstNUL(&row, &col, src, fsrcSize);
            free(buf);
            handleExit(EMBEDDED_NUL_ERROR, FILENAME, row, col, 0, EMPTY_STR);
        }

        buf[fsize] = '\0';

        Lexer lexer = createLexer(src, FILENAME);
        Token t = nextToken(&lexer);
        printToken(&t);

        // Print tokens - temporarily automatic
        while (t.kind != NON_SOURCE) {
            t = nextToken(&lexer);
            printToken(&t);
        }

        free(buf);
    } else {
        handleExit(USAGEREQ, EMPTY_STR, NO_ROWCOL_GIVEN, 0, EMPTY_STR);
    }

    handleExit(SUCCESS, EMPTY_STR, NO_ROWCOL_GIVEN, 0, EMPTY_STR);
}
