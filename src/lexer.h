/**
 * @file lexer.h
 * @brief Lexer header file defining enums and structs for the lexer
 */

#ifndef CHASM_LEXER_H
#define CHASM_LEXER_H

#include <stdlib.h>

#define KEYWORD_COUNT 70

typedef enum {
    IDENTIFIER,      // Letters, digits, and _, not starting with a digit, case-sensitive
    KEYWORD,         // Declarations, control flow, errors, memory, values, other keywords, types, and directives:
                     //   Declarations: var, const, unsigned, define, lambda, struct, enum, error, interface, public, private, self
                     //   Control Flow: if, other, for, foreach, in, while, do, stop, continue, break, travel, switch, is, otherwise, return, leaveprog, block
                     //         Errors: attempt, reject, err, of
                     //         Memory: new, del, defer, sizeof
                     //         Values: true, false, NULL, undefined
                     //          Other: asm
                     //          Types: void, bool, char, unichar, byte, dbyte, string, int, int8, int16, int32, int64, short, long, float,
                     //               | float32, float64, double, double32, double64, szt, pszt
                     //     Directives: get, as, use, else, end, bare (define and if above)
    NUM_LITERAL,     // Decimal, hex, binary, octal, and float
                     //   f, l, s, z, u - combinable in any order; `f` only allowed on float literals, because it is a hex digit;
                     //   repeating suffixes are errors
    CHAR_LITERAL,    // 'a' and '\u{1F600}' as well as escapes:
                     //   Escape Codes: \n \t \r \0 \\ \' \" \xHH \u{H...}
    STR_LITERAL,     // "..." which may span lines; 
                     //   strings use the same escapes as CHAR_LITERAL, one bad escape is INVALID; unclosed string is Error.0016
    PUNCTUATION,     // Brackets, arithmetic, incrementors, bitwise, comparisons, logical, ternary, assignment, prefix, hash prefixes (with ^^(hashForDoubleCaretDef))
                     //       Brackets: (, ), [, ], {, }, ,, ;, ., :
                     //     Arithmetic: +, -, *, /, %
                     //   Incrementors: ++, --
                     //        Bitwise: &, |, ^, ~, <<, >>
                     //     Comparison: <, <=, >, >=, ==, !=
                     //        Logical: !, &&, ||
                     //        Ternary: =>
                     //     Assignment: =, +=, -=, *=, /=, %=, <<=, >>=, &=, ^=, |=
                     //         Prefix: ^^
                     //    Hash Prefix: # (only with ^^(hashForDoubleCaretDef))
    NON_SOURCE,      // EOF token for the parser to stop on
    INVALID,         // Malformed numbers, characters, or string literals, covering the whole token; stray characters will give Error.0017
    NEWLINE,         // Specifically for newline characters
} TokenKind;

typedef enum {
    // KW_*          // kw length
    // -- Declarations
    KW_VAR,          // var 3
    KW_CONST,        // const 5
    KW_UNSIGNED,     // unsigned 8
    KW_DEFINE,       // define 6
    KW_LAMBDA,       // lambda 6
    KW_STRUCT,       // struct 6
    KW_ENUM,         // enum 4
    KW_ERROR,        // error 5
    KW_INTERFACE,    // interface 9
    KW_PUBLIC,       // public 6
    KW_PRIVATE,      // private 7
    KW_SELF,         // self 4
    // -- Control Flow
    KW_IF,           // if 2
    KW_OTHER,        // other 5
    KW_FOR,          // for 3
    KW_FOREACH,      // foreach 7
    KW_IN,           // in 2
    KW_WHILE,        // while 5
    KW_DO,           // do 2
    KW_STOP,         // stop 4
    KW_CONTINUE,     // continue 8
    KW_BREAK,        // break 5
    KW_TRAVEL,       // travel 6
    KW_SWITCH,       // switch 6
    KW_IS,           // is 2
    KW_OTHERWISE,    // otherwise 9
    KW_RETURN,       // return 6
    KW_LEAVEPROG,    // leaveprog 9
    KW_BLOCK,        // block 5
    // -- Error Handling
    KW_ATTEMPT,      // attempt 7
    KW_REJECT,       // reject 6
    KW_ERR,          // err 3
    KW_OF,           // of 2
    // -- Memory Management
    KW_NEW,          // new 3
    KW_DEL,          // del 3
    KW_DEFER,        // defer 5
    KW_SIZEOF,       // sizeof 6
    // -- Values
    KW_TRUE,         // true 4
    KW_FALSE,        // false 5
    KW_NULL,         // NULL 4
    KW_UNDEFINED,    // undefined 9
    // -- Other
    KW_ASM,          // asm 3
    // -- Types
    KW_VOID,         // void 4
    KW_BOOL,         // bool 4
    KW_CHAR,         // char 4
    KW_UNICHAR,      // unichar 7
    KW_BYTE,         // byte 4
    KW_DBYTE,        // dbyte 5
    KW_STRING,       // string 6
    KW_INT,          // int 3
    KW_INT8,         // int8 4
    KW_INT16,        // int16 5
    KW_INT32,        // int32 5
    KW_INT64,        // int64 5
    KW_SHORT,        // short 5
    KW_LONG,         // long 4
    KW_FLOAT,        // float 5
    KW_FLOAT32,      // float32 7
    KW_FLOAT64,      // float64 7
    KW_DOUBLE,       // double 6
    KW_DOUBLE32,     // double32 8
    KW_DOUBLE64,     // double64 8
    KW_SZT,          // szt 3
    KW_PSZT,         // pszt 4
    // -- Directives
    KW_GET,          // get 3
    KW_AS,           // as 2
    KW_USE,          // use 3
    KW_ELSE,         // else 4
    KW_END,          // end 3
    KW_BARE,         // bare 4
} KeywordKind;

typedef enum {
    // -- Brackets and Separators
    PC_LPAREN,       // (
    PC_RPAREN,       // )
    PC_LSQUARE,      // [
    PC_RSQUARE,      // ]
    PC_LBRACE,       // {
    PC_RBRACE,       // }
    PC_COMMA,        // ,
    PC_SEMICOLON,    // ;
    PC_DOT,          // .
    PC_COLON,        // :
    // -- Arithmetic
    PC_PLUS,         // +
    PC_MINUS,        // -
    PC_TIMES,        // *
    PC_DIVIDE,       // /
    PC_MOD,          // %
    // -- Increment
    PC_INCREMENT,    // ++
    PC_DECREMENT,    // --
    // -- Bitwise
    PC_BITAND,       // &
    PC_BITOR,        // |
    PC_BITXOR,       // ^
    PC_BITNOT,       // ~
    PC_SHIFTLEFT,    // <<
    PC_SHIFTRIGHT,   // >>
    // -- Comparison
    PC_LT,           // <
    PC_LTEQUAL,      // <=
    PC_GT,           // >
    PC_GTEQUAL,      // >=
    PC_EQUAL,        // ==
    PC_NEQUAL,       // !=
    // -- Logical
    PC_NOT,          // !
    PC_AND,          // &&
    PC_OR,           // ||
    // -- Ternary
    PC_TERNARY,      // =>
    // -- Assignment
    PC_ASSIGN,       // =
    PC_PLUSASSIGN,   // +=
    PC_MINUSASSIGN,  // -=
    PC_TIMESASSIGN,  // *=
    PC_DIVIDEASSIGN, // /=
    PC_MODASSIGN,    // %=
    PC_LEFTASSIGN,   // <<=
    PC_RIGHTASSIGN,  // >>=
    PC_BITANDASSIGN, // &=
    PC_BITXORASSIGN, // ^=
    PC_BITORASSIGN,  // |=
    // -- Prefix
    PC_DIRECTIVE,    // ^^
    // -- Hash Prefix
    PC_HASHDIRECTIVE,// #
} PunctuationKind;

// Stores a keyword for the below keyword storage
typedef struct {
    const char* text;
    size_t length;
    KeywordKind kw;
} Keyword;

// Defined in keywords.c
extern const Keyword KEYWORD_LIST[KEYWORD_COUNT];
extern const char* KEYWORD_ENUM_CONVERSION_TABLE[KEYWORD_COUNT];

typedef struct {
    TokenKind kind;         // Token type
    // This union is useless unless the token is a keyword or punctuation, otherwise it is NULL, or reads garbage
    union {                 // What it is, if applicable (ex. PC_TERNARY or KW_IF) otherwise void* to NULL
        KeywordKind kw;
        PunctuationKind pc;
        void* n;
    } typeValue;

    const char* text;       // Pointer to the text - not terminated with \0, use length below
    size_t length;          // Use length to avoid allocating memory (and therefore freeing) for the text

    size_t row;             // Row and column for errors
    size_t col;
} Token;

typedef struct {
    const char* c;      // A pointer to the current character
    size_t row;         // Row and column to pass to tokens
    size_t col;
    const char* file;   // Filename for error handling
} Lexer;

Lexer createLexer(const char* src, const char* filename);
Token nextToken(Lexer* lexer);

void printToken(Token* tok);

#endif