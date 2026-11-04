# Chasm

Chasm is a hand-coded compiled language loosely based on C and C++ with unique syntax, manual memory management, and x86-64 NASM output. `cmc` is the compiler, written in C.

![MIT License](https://img.shields.io/badge/license-MIT-green)
![C11](https://img.shields.io/badge/C-11-blue)
![Status](https://img.shields.io/badge/status-lexer_stage-orange)
![Development](https://img.shields.io/badge/development-in_progress-yellow)
![Compiler](https://img.shields.io/badge/compiler-cmc_--_written_in_C-blue)

## Status

Chasm is currently in the lexer stage - it only lexes tokens for now.

### What's done (lexer)
- identifiers
- keywords
- punctuation
- comments
- decimal numbers
- an error handling system
- prefix numbers (`0x`, `0b`, `0o`)

### What's to come (lexer)
- characters
- strings

### What's to come (beyond)
- parsing
- x86-64 NASM generation

## Build

### Syntax Check

Run `make check` in your terminal to check syntax.
Example run:

```
--------------------------------
Checking syntax for:
src/errors.h src/lexer.h
--------------------------------
gcc -Wall -Wextra -std=c11 -Wshadow -Wformat=2 -Wundef -Werror -fsyntax-only src/errors.h src/lexer.h


--------------------------------
Checking syntax for:
src/cmc.c src/errors.c src/keywords.c src/lexer.c
--------------------------------
gcc -Wall -Wextra -std=c11 -Wshadow -Wformat=2 -Wundef -Werror -fsyntax-only src/cmc.c src/errors.c src/keywords.c src/lexer.c
Syntax check completed
```

### Debug Build

Run `make debug` in your terminal to compile a debug build (`-O0`).
Example run:

```
--------------------------------
Compiling executable (debug)
--------------------------------

gcc -Wall -Wextra -std=c11 -Wshadow -Wformat=2 -Wundef -Werror -O0 -ggdb  src/cmc.c src/errors.c src/keywords.c src/lexer.c -o cmc.exe
```

### Release Build

Run `make release` in your terminal to compile a release build (`-O2`).
Example run:

```
--------------------------------
Compiling executable (release)
--------------------------------

gcc -Wall -Wextra -std=c11 -O2 src/cmc.c src/errors.c src/keywords.c src/lexer.c -o cmc.exe
```

### Clean

Run `make clean` to remove all executable builds.
Example run:

```
Cleaning .gch files...
rm -f src/*.gch
All .gch files deleted


Cleaning executables...
rm -f chasm chasm.exe
All executables deleted
```

### Werror

`-Werror` is on by default via the `USE_WERROR` Makefile flag near the top of the Makefile.

### Windows

To build on Windows, you need MSYS2 with `mingw32-make`, with `C:\msys64\usr\bin` on `PATH`. There are no sanitizers there, so use WSL for those.

## Usage

Running `cmc` with no arguments currently gives this output:
```
cmc <file.cm>
```

This, for now, only prints every token with a success message. This type of output will soon be condensed into a `--tokens` flag.

## Project Structure

```
docs/
    chasm_spec.md
src/
    cmc.c
    errors.c
    errors.h
    keywords.c
    lexer.c
    lexer.h
tests/
    chasm/
        bad_char_backslash.cm
        bad_char_backtick.cm
        bad_char_control.cm
        bad_char_dollar.cm
        bad_char_form_feed.cm
        bad_char_hash.cm
        bad_char_later_line.cm
        bad_char_non_ascii.cm
        bad_char_question.cm
        bom_at_start.cm
        bom_not_at_start.cm
        broken_string.cm
        characters.cm
        comments_utf8.cm
        comments.cm
        double_caret_to_caret.cm
        embedded_nul.cm
        hello_world.cm
        hex_to_float_suffix.cm
        identifiers_keywords.cm
        numbers.cm
        prefix_test.cm
        sample_file.cm
        strings_multiline.cm
        strings.cm
        suffix_test.cm
        test_octal_notation_0o.cm
        unclosed_comment_after_closed.cm
        unclosed_comment.cm
./
    .gitignore
    LICENSE
    Makefile
    README.md
```

## Tests

Each `.cm` test file in `tests/chasm/*` ends with a `/* Expected output: */` block for token output. Each line is formatted with `text Kind`. There is not yet an automated runner for these tests, so they must be run manually with the `cmc` compiler.

## Specification

See [`docs/chasm_spec.md`](./docs/chasm_spec.md).

## License

MIT - see [`LICENSE`](./LICENSE).
