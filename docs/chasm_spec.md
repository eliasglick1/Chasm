# Chasm Language Specification

## Table of Contents

- [Overview](#overview)
- [Lexical Structure](#lexical-structure)
    - [Comments](#comments)
    - [Identifiers](#identifiers)
    - [Keywords](#keywords)
    - [Number Literals](#number-literals)
    - [Character and String Literals](#character-and-string-literals)
    - [Directive Prefix](#directive-prefix)
- [Types](#types)
    - [Primitive Types](#primitive-types)
    - [Built-in Aliases](#built-in-aliases)
    - [Pointers](#pointers)
    - [Arrays](#arrays)
    - [Strings](#strings)
- [Variables and Constants](#variables-and-constants)
    - [`const`](#const)
    - [`^^define` (Text Macros)](#define-text-macros)
- [Operators](#operators)
    - [Precedence (Highest First)](#precedence-highest-first)
    - [Rules](#rules)
    - [Conversions](#conversions)
- [Control Flow](#control-flow)
    - [Branches](#branches)
    - [Loops](#loops)
    - [Switch](#switch)
    - [Anonymous Blocks](#anonymous-blocks)
- [Functions](#functions)
    - [Function Pointers](#function-pointers)
    - [Lambdas](#lambdas)
- [Structs and Enums](#structs-and-enums)
    - [Structs](#structs)
    - [Enums](#enums)
    - [No Unions](#no-unions)
- [Interfaces and Generics](#interfaces-and-generics)
    - [Interfaces](#interfaces)
    - [Generics](#generics)
    - [Built-in Interfaces (in `core`)](#built-in-interfaces-in-core)
- [Errors](#errors)
- [Memory Management](#memory-management)
    - [`new` and `del`](#new-and-del)
    - [`defer`](#defer)
    - [Allocators](#allocators)
- [Directives and Modules](#directives-and-modules)
    - [Imports](#imports)
    - [Aliases and Macros](#aliases-and-macros)
    - [Conditional Compilation](#conditional-compilation)
    - [Attributes](#attributes)
- [Program Entry, Runtime and Inline Assembly](#program-entry-runtime-and-inline-assembly)
    - [Runtime](#runtime)
    - [Inline Assembly](#inline-assembly)
- [Standard Library](#standard-library)
- [The `cmc` Toolchain](#the-cmc-toolchain)
    - [Tests](#tests)
- [Idiomatic Practices](#idiomatic-practices)
    - [Memory](#memory)
    - [Arrays and Pointers](#arrays-and-pointers)
    - [Errors](#errors-1)
    - [Operators and Expressions](#operators-and-expressions)
    - [Files and Naming](#files-and-naming)
- [Quirks](#quirks)
    - [Types and Declarations](#types-and-declarations)
    - [Operators](#operators-1)
    - [Control Flow](#control-flow-1)
    - [Memory, Errors and Modules](#memory-errors-and-modules)

## Overview

Chasm is a compiled, C-family systems language with manual memory management, a large standard library (STD) and no libc dependency. Source files use the `.cm` extension. This language is a work in progress and not everything in this document is implemented yet.

**Design pillars**

- **Familiar to C programmers**, with the worst traps fixed: `int* a, b;` declares two pointers, arrays never decay, and there is no `->` operator.
- **Visible memory:** every allocation is a `new`, every free is a `del`, and `defer` makes cleanup sit next to allocation. Nothing is freed behind your back.
- **Errors are values.** A function that can fail returns `T!`; `attempt` / `reject` handle failures without exceptions or stack unwinding.
- **Structs + interfaces + generics**, with no classes, no inheritance, no unions and no runtime interface values.
- **Directives are explicit:** anything that bends the language (imports, macros, aliases, opt-in quirks) starts with `^^`.

**Toolchain pipeline**

1. `cmc` (the Chasm compiler, written in C) parses `.cm` files and emits x86-64 NASM assembly.
2. NASM assembles the `.asm` into object files.
3. A linker produces the executable. On Linux the runtime uses raw syscalls; on Windows it imports `kernel32.dll` (`VirtualAlloc`, `WriteFile`, `ExitProcess`, …), because Windows syscall numbers are undocumented and change between versions.
4. Long-term goal: rewrite `cmc` in Chasm and compile it with the C version.

**Hello world**

```cm
^^get io

define ^int main() {
  io.println("Hello, Chasm!");
  return 0;
}
```

## Lexical Structure

Chasm's tokens follow C closely, with added literal prefixes, type suffixes and strings that may span lines.

### Comments

```cm
// line comment
/* block comment */
/* block comments do NOT nest: /* this */ ends the comment here <-- */
```

### Identifiers

Letters, digits and `_`, ASCII only, not starting with a digit. Case-sensitive. Keywords cannot be used as identifiers; with `^^(tfAliasDef)` active, neither can `t` or `f`.

### Keywords

| Group | Keywords |
| --- | --- |
| Declarations | `var` `const` `unsigned` `define` `lambda` `struct` `enum` `error` `interface` `public` `private` `self` |
| Control flow | `if` `other` `for` `foreach` `in` `while` `do` `stop` `continue` `break` `travel` `switch` `is` `otherwise` `return` `leaveprog` `block` |
| Errors | `attempt` `reject` `err` `of` |
| Memory | `new` `del` `defer` `sizeof` |
| Values | `true` `false` `NULL` `undefined` |
| Other | `asm` |
| Built-in types | `void` `bool` `char` `unichar` `byte` `dbyte` `string` `int` `int8` `int16` `int32` `int64` `short` `long` `float` `float32` `float64` `double` `double32` `double64` `szt` `pszt` |
| Directive keywords (after `^^`) | `get` `as` `use` `define` `if` `else` `end` `bare` |

Most directive keywords are reserved everywhere in the file.

### Number Literals

| Form | Example | Value |
| --- | --- | --- |
| Decimal | `42`, `010` | 42, 10 (a leading zero does not mean octal) |
| Hexadecimal | `0xFF`, `0x1f` | 255, 31 |
| Binary | `0b1010` | 10 |
| Octal | `0o17` | 15 |
| Float | `1.5`, `2.0e10` | 1.5, 20,000,000,000 (float64 by default) |

There are no digit separators (such as `_`).

**Optional type suffixes** (combinable, e.g. `5ul`):

| Suffix | Type | Note |
| --- | --- | --- |
| `f` | `float` | Only allowed on float literals such as `1.5f`. Never on hex, where `f` is a digit |
| `l` | `long` | Defined as int64 |
| `s` | `short` | Defined as int8 |
| `z` | `szt` | Equivalent of `size_t` in C |
| `u` | `unsigned` | - |

Suffixes may appear in any order (`5ul` and `5lu` are the same), and repeating one suffix (`5uu`) is an error. `f` is only allowed on float literals (`1.5f`), never on integer forms. A float needs digits on both sides of the dot, so `1.`, `.5` and `1e5` are not legal. An exponent is a lowercase `e`, an optional `+` or `-`, and digits (`2.0e10`, `1.5e-3`); `E` is not legal. Letters may not follow a literal (`123abc` is an error). All suffixes are lowercase (no `F`, `L`, `S`, `Z`, `U`).

### Character and String Literals

```cm
char c = 'a';
unichar e = '\u{1F600}';
string s = "tab\there\n";
string multi = "line one
line two";            // a string runs until the next unescaped "
```

Escapes: `\n` `\t` `\r` `\0` `\\` `\'` `\"` `\xHH` `\u{H…}`. A string literal is stored in read-only memory as a zero-terminated `char` array.

Strings use the same escapes as characters. A string may contain raw line breaks, which stay exactly as written (a file saved with CRLF line endings gives a string containing `\r\n`), and a backslash directly before a line break is not an escape. A string may contain only ASCII characters; write any other character with `\u{H...}`. A string with a bad escape or a non-ASCII byte is malformed (the parser reports it), and a string that is never closed is a compile error (`Error.0016`) reported at its opening `"`.

### Directive Prefix

`^^` starts a directive (`^^get`, `^^define`, `^^if`) or an attribute (`^^(name)`). Directives take no semicolon. After `^^(hashForDoubleCaretDef)`, `#` may be written in place of `^^` for the rest of the file.

## Types

Every built-in type has a fixed size on every platform, except `pszt`, which matches the pointer size.

### Primitive Types

| Type | Size | Notes |
| --- | --- | --- |
| `bool` | 1 byte | `true` / `false`; `t` / `f` with `^^(tfAliasDef)` |
| `char` | 1 byte | ASCII |
| `unichar` | 4 bytes | One Unicode code point |
| `byte` | 1 byte | Unsigned; same as `unsigned int8` |
| `dbyte` | 2 bytes | Unsigned; same as `unsigned int16` |
| `int8` `int16` `int32` `int64` | 1–8 bytes | Signed, two's complement |
| `unsigned intN` | same as `intN` | `unsigned` is a separate prefix keyword |
| `float32` `float64` | 4, 8 bytes | IEEE 754. There are no 16-bit or 128-bit floats |
| `double32` `double64` | 4, 8 bytes | Same IEEE 754 formats as `float32` / `float64`, under separate names |
| `szt` | 8 bytes | Always `unsigned int64`, on every target |
| `pszt` | pointer size | Signed; for pointer differences |
| `void` | — | Only as a return type (`^void`) or in `void*` |

### Built-in Aliases

| Alias | Value |
| --- | --- |
| `int` | `int32` |
| `short` | `int8` |
| `long` | `int64` |
| `float` | `float64` |
| `double` | `double64` |
| `string` | `char*`, and `string[]` means `char[]` (see Strings) |

User aliases use `^^use int as num`.

### Pointers

```cm
int* a, b;          // BOTH are int*
int x = 5;
int* p = &x;        // & takes an address
*p = 6;             // * dereferences
p = p + 1;          // pointer arithmetic is allowed
Node* head = NULL;  // the null pointer is NULL
const int* r = &x;  // read-only data; `int const*` means the same
```

- A pointer itself is always reassignable; there is no "constant pointer".
- `.` reaches struct members through one level of pointer: `head.next.value`. A `Node**` needs `(*pp).value`.

### Arrays

```cm
int32[4] a = {1, 2, 3, 4};
int32[4] b = {1, 2};          // remaining elements are zero
int32[3] c = {1, 2, 3, 4};    // compile error: too many values
int32[] d = {1, 2, 3};        // size inferred: int32[3]
szt n = d.len;                // 3
int32* p = d.ptr;             // explicit pointer to the first element
szt bytes = sizeof(a);        // 16, the whole array
```

- Arrays are values. They **never** decay to pointers, and passing one to a function copies it. The compiler warns when a copied array is over about 256 bytes.
- Every array has built-in `.len` and `.ptr`.
- There are no slices and **no bounds checking**, in any build mode.
- **Runtime-sized arrays** live on the heap and are typed `T[]*`. Their length is stored in a hidden header before the data, so `.len` works on them too:

```cm
int[]* buf = new[count] ^int;
szt n = buf.len;
```

### Strings

- `string` is `char*`, a pointer to zero-terminated characters, as in C.
- `string[]` means `char[]`, a character buffer. It is **not** an array of strings. An array of strings is written `char*[]`.
- String literals give a pointer to a read-only, zero-terminated array. This is the one place an array becomes a pointer automatically.
- `^^(stringAliasUndef)` removes the `string` keyword for the whole file.
- The STD's growable, owned `String` type (similar to C++'s `std::string`) lives in the `str` module.

```cm
string name = "Chasm";          // char*
string[] buf = "hi";            // char[3]: 'h', 'i', '\0'
char*[] names = {"a", "b"};     // array of 2 strings
```

## Variables and Constants

A declaration is `type name = value;`. Leaving out the value zeroes the variable, and `undefined` skips initialization for speed.

```cm
int32 count = 0;
var name = "Chasm";            // type inferred from the right side
unsigned int64 total;          // no '=' means zeroed
int32[4096] buf = undefined;   // left as garbage on purpose
```

### `const`

```cm
const int MAX = 100;
const int SIZE = 4 * 1024;     // computed at compile time
int32[SIZE] page;              // usable as an array size
```

Constants are typed and evaluated while compiling when possible. Function parameters are ordinary, changeable variables.

### `^^define` (Text Macros)

`^^define` pastes text, like C's `#define`. Use `const` when you want a typed value.

```cm
^^define BUFSIZE as 512
^^define SQ(x) as x * x        // SQ(1+2) expands to 1+2*1+2 = 5
^^define SQ2(x) as ((x) * (x)) // safe form: SQ2(1+2) = 9
```

## Operators

Chasm keeps C's operators and C's precedence. The differences: the ternary is `cond => a : b`, only postfix `++`/`--` exist, and arithmetic always wraps.

### Precedence (Highest First)

| Level | Operators | Associativity |
| --- | --- | --- |
| 1 | `()` call, `[]` index, `.` member, `x++`, `x--`, `:<T>` explicit generic | left |
| 2 | unary `+` `-` `!` `~`, `*` deref, `&` address, `(T)` cast, `sizeof` | right |
| 3 | `*` `/` `%` | left |
| 4 | `+` `-` | left |
| 5 | `<<` `>>` | left |
| 6 | `<` `<=` `>` `>=` | left |
| 7 | `==` `!=` | left |
| 8 | `&` | left |
| 9 | `^` | left |
| 10 | `\|` | left |
| 11 | `&&` | left |
| 12 | `\|\|` | left |
| 13 | `=> :` ternary | right |
| 14 | `=` `+=` `-=` `*=` `/=` `%=` `<<=` `>>=` `&=` `^=` `\|=` | right |
| 15 | `a, b` comma: run a, discard it, result is b (as in C) | left |

Because comparisons bind tighter than bitwise operators (as in C), `x & 1 == 0` means `x & (1 == 0)`. Write `(x & 1) == 0`.

### Rules

- **Increment:** `x++` and `x--` only, returning the old value. `++x` is a compile error.
- **Assignment in conditions is allowed:** `if (x = read())` assigns, then tests the value.
- **Ternary:** `int m = (a > b) => a : b;`. The `=>` token is reserved for this.
- **Overflow always wraps** (two's complement), signed and unsigned. `int8` 127 + 1 is −128. Checked and saturating math lives in `math`.
- **Shifts:** a shift by a constant ≥ the type's width is a compile error. With `^^(defaultBitShiftDef)`, the count is masked to its low bits instead (6 bits for 64-bit values, as x86-64 does). `>>` is arithmetic on signed types and logical on unsigned.
- **Integer division by zero** crashes the program: x86-64 `div` raises a CPU exception.

### Conversions

| From → to | Rule |
| --- | --- |
| Integer → integer (same signedness) | Automatic, both widening and narrowing. Narrowing silently drops the high bits, as in C |
| Signed ↔ unsigned mixed in one expression | Compile error; cast one side |
| Integer → float | Automatic. `int64` values above 2^53 may be rounded |
| Float → integer | Requires a cast: `(int)f` |
| Float → smaller float | Requires a cast |
| Enum → integer | Requires a cast: `(short)lvl` |
| Float ↔ double (any width, even the same) | Requires a cast: `(double64)f` |

Casts use C syntax, `(T)x`. A `cast` function arrives in the STD, and `bitcast` reinterprets raw bits.

## Control Flow

Braces are optional when a body is a single statement, as in C. `other` replaces `else`, `stop` leaves one loop, and `break` leaves every loop at once.

### Branches

```cm
if (a) {
  ...
} other if (b) {
  ...
} other {
  ...
}
```

A single-statement body may drop its braces. As in C, an `other` belongs to the nearest unmatched `if`. Switch case bodies still always need braces.

```cm
if (x > 0) io.println("positive");
other io.println("not positive");

for (int i = 0; i < n; i++) total += a[i];
while (p != NULL) p = p.next;
```

### Loops

```cm
for (int i = 0; i < n; i++) { ... }
foreach (int x in arr) { ... }       // fixed arrays and T[]* arrays
while (cond) { ... }
do { ... } while (cond);
```

`foreach` can also give the index, counting from 0. The index comes first and is any integer type, usually `szt`:

```cm
foreach (szt i, int x in arr) {
  io.println(i, ": ", x);
}
```

| Statement | Effect |
| --- | --- |
| `stop;` | Leave the innermost loop |
| `continue;` | Jump to the next iteration of the innermost loop |
| `break;` | Leave **all** enclosing loops in the current function |
| `travel name;` | Jump to `name:` in the same function (C's `goto`) |

`^^(gotoDef)` also accepts the spelling `goto`.

When `travel` jumps out of a block, that block's `defer`s run first. Jumping *into* a block past a variable declaration is a compile error, since the variable would never be set up.

```cm
foreach (int row in rows) {
  foreach (int col in cols) {
    if (grid[row][col] == target) { break; }  // exits both loops
  }
}

retry:
if (!ready()) { travel retry; }
```

### Switch

```cm
switch (v) {
  is 1, 2, 3: { io.println("small"); }
  is 4: { io.println("four"); }
  otherwise: { io.println("other"); }
}
```

- Each case body needs braces.
- There is **no fallthrough** and no `stop` is needed.
- Inside a loop: `stop` leaves the switch, `continue` goes to the loop's next iteration, and `break` leaves every loop.

### Anonymous Blocks

A bare scope block needs the `block` keyword. Its `defer`s run at its closing brace.

```cm
block {
  Node* tmp = new Node {val: 1};
  defer del tmp;
  ...
}
```

## Functions

Functions start with `define`, and `^` marks the return type. Overloads and default arguments are allowed; references are not.

```cm
define ^int add(int a, int b) { return a + b; }
define ^void log(string msg) { ... }
define ^int! parse(string s) { ... }                 // may fail
define ^void open(string path, bool create: false) { ... }  // default argument
```

- **Default arguments:** use `name: value` and parameters with defaults must come after all parameters without defaults.
- **Overloading:** functions may share a name if their parameter types differ. A call that matches two overloads equally well is a compile error. Overloads get mangled assembly labels.

**Name mangling:** Assembly labels must be unique, so the compiler renames every function as `module$name$paramtypes`. For example, `print(int)` in `io` becomes `io$print$int`, and `max:<int>` in `math` becomes `math$max$int`. Pointer types are spelled with a `p` suffix (`charp`). `main` is never mangled. If two different functions would produce the same label, it is a compile error.

```cm
define ^void bump(int* x) { *x = *x + 1; }
bump(&count);
```

### Function Pointers

```cm
^int(int, int) op = add;
int r = op(2, 3);
```

### Lambdas

```cm
var sq  = lambda ^int (int x) x * x;                // one expression: no braces, its value is returned
var sum = lambda ^int (int a, int b) {
  int s = a + b;
  return s;
};
var sqc = lambda ^int |tosq| (int x: tosq) x * x;   // captures tosq; it becomes x's default
```

- A lambda without captures converts to a plain function pointer.
- `|name|` captures **by copy**, stored inside the lambda value on the stack. There is no hidden heap allocation.
- `|&name|` captures a pointer instead. It dangles if the lambda outlives the variable.
- A capturing lambda carries data, so it **cannot** be stored in a plain function-pointer type.

## Structs and Enums

Structs hold fields and methods, members are public by default, and fields keep the order you write them. Chasm has no classes, inheritance or unions.

### Structs

```cm
struct Rect {
  int w;
  int h;
  private int id;
  public int tag;                     // `public` may be written explicitly

  define ^int area() { return self.w * self.h; }
  define ^void grow(int by) { self.w = self.w + by; }
}

Rect r = {w: 3, h: 4};                 // named fields
Rect q = {3, 4};                       // by position
Rect z;                                // all fields zeroed
int a = r.area();
```

- Methods are written inside the struct. `self` is automatic and is a pointer to the struct (`Rect*`).
- `private` members are visible only to the struct's own methods.
- Fields are laid out in written order with natural padding. `^^(packStructs)` removes padding for the file.

### Enums

```cm
enum ^short Level {     // underlying type specifier is not required; defaults to int32
  Level.LOW,            // 0
  Level.MED,            // 1
  Level.HIGH = 10,      // explicit value
}                       // trailing comma allowed

Level lv = Level.MED;
short raw = (short)lv;  // converting to an integer needs a cast
```

- `^type` sets the underlying integer type.
- Members are always scoped (`Level.LOW`), even inside the declaration.
- Values count up from 0 unless set.

### No Unions

Chasm has no `union` keyword like C. Reinterpreting bits uses `bitcast(^int64, someFloat)`.

## Interfaces and Generics

Interfaces only constrain generics; there are no interface values or vtables, so every call is resolved at compile time.

### Interfaces

```cm
interface Shape {
  define ^int area();
}

struct Square &= Shape, Printable {   // &= means "implements"
  int side;
  define ^int area() { return self.side * self.side; }
  define ^void print() { ... }
}
```

A struct must list every interface it implements, and the compiler checks that each required method exists.

### Generics

```cm
define ^T max<T: Comparable>(T a, T b) { return (a > b) => a : b; }

struct List<T: Any> {
  T* items;
  szt count;
}

int m  = max(3, 7);              // T inferred
int m2 = max:<int>(a, b);        // explicit
List<int> nums;
```

- Every type parameter **must** name an interface. `Any` accepts every type.
- Each type a generic is used with gets its own copy in the assembly (monomorphization, as with C++ templates). This is fast but makes the program bigger.

### Built-in Interfaces (in `core`)

| Interface | Means | Built-in types implementing it |
| --- | --- | --- |
| `Any` | No requirements | All |
| `Equatable` | Supports `==` and `!=` | All numbers, `bool`, `char`, `unichar`, pointers, enums |
| `Comparable` | Supports `<` `<=` `>` `>=` | All numbers, `char`, `unichar` |
| `Numeric` | Supports arithmetic operators | Integer and float types |
| `Allocator` | Can serve `new` / `del` | `Heap`, `Arena`, `Pool`, `FixedBuffer`, `DebugAlloc` |

Without interface values, a list mixing different shapes is built by hand from function pointers, as in C.

## Errors

Errors are enum values returned alongside normal results. `attempt` / `reject` handle them; the compiler adds a hidden check after each failing call, so nothing is thrown and no stack is unwound.

```cm
error FileError {
  FileError.NotFound,
  FileError.Denied,
}

define ^File! open(string path) {          // ! = may return any error
  ...
  return FileError.NotFound;
}

define ^File!FileError openStrict(string p) { ... }  // only FileError

File f = undefined;
attempt {
  f = open("a.txt");
  string text = f.readAll();
} reject (err e of FileError) {
  io.println("file problem");
} reject (err e of NetError) {
  io.println("network problem");
}
```

- A `!` call **outside** an `attempt` block is a compile error.
- When a call inside `attempt` fails, control jumps to the first `reject` whose error type matches.
- An error no `reject` matches passes up to the caller. This is only allowed when the current function is itself `!`; otherwise it is a compile error.
- Ignoring or discarding a `T!` result is a compile error.
- Variables declared inside `attempt` end at its closing brace. Declare them before the block to use them afterward.
- `defer`s inside the `attempt` block still run when control jumps to `reject`.
- `main` is implicitly `!`. An error that reaches it prints its name (for example `FileError.NotFound`) and exits with code 1.

## Memory Management

Memory is fully manual: `new` allocates, `del` frees, and `defer` schedules a statement for the end of the enclosing block. There is no garbage collector and no destructors.

### `new` and `del`

```cm
Node* n = new Node {val: 3};        // one object; NULL if allocation fails
if (n == NULL) { return 1; }
defer del n;

int[100]* fixed = new[100] ^int;    // constant count: typed int[100]*
int[]* buf = new[count] ^int;       // runtime count: typed int[]*, buf.len works
defer del buf;                      // one del for objects and arrays
```

- `new` returns `NULL` on failure.
- `del` handles single objects and arrays alike. Array allocations store their count in a header, so `del` knows the size.
- `del` on an allocator frees **everything** it handed out (see Allocators).

`del NULL;` does nothing, as with C's `free(NULL)`. So you can always write `del p;` without first checking `if (p != NULL)`, which keeps cleanup code short when a pointer may never have been allocated.

### `defer`

- Runs when the enclosing **block** ends, including through `return`, `stop`, `break`, `travel` or a jump to `reject`.
- Several defers in one block run in reverse order, last written first.
- There is no `errdefer`.

The one exception is `leaveprog`, which exits the program at once and skips every pending `defer`.

```cm
block {
  File a = ...; defer a.close();
  File b = ...; defer b.close();
}   // b closes, then a
```

### Allocators

An allocator is the object that actually hands out memory. `new` uses the default `heap` unless told otherwise.

| Allocator | How it works | Good for |
| --- | --- | --- |
| `Heap` (default `heap`) | Gets memory from the OS (`mmap` / `VirtualAlloc`) and tracks each block | General use; slowest |
| `Arena` | Bumps a pointer through one big chunk; individual `del`s do nothing | Batch lifetimes: a compiler pass, a frame, a request |
| `Pool` | Hands out equal-size blocks from a free list | Many identical objects, such as list nodes |
| `FixedBuffer` | Allocates from memory you already own, even a stack array | Zero OS calls |
| `DebugAlloc` | Wraps another allocator and logs every `new` / `del` | Reporting leaks and double frees at exit |

```cm
Arena ar = Arena.create(1024 * 1024);
defer del ar;                         // frees the whole arena at once

Node* a = new Node {val: 1};          // default heap
Node* b = new(ar) Node {val: 2};      // from the arena; no individual del

define ^List<T> make<T: Any>(Allocator al: heap) { ... }  // allocator as a default argument
```

All allocator types live in `core`, so they need no import. Because interfaces are compile-time only, a function taking an `Allocator` is compiled once per allocator type.

## Directives and Modules

Everything starting with `^^` is a directive: imports, aliases, macros, conditional compilation and attributes. Directives take no semicolon.

### Imports

```cm
^^get io                    // STD module io, used as io.println(...)
^^get math as stdMath       // renamed prefix
^^get math.cm as myMath     // one file as a module
^^get ./geometry            // always your own folder
^^get io as bare            // no prefix: println(...)
```

**How `^^get name` is resolved:**

1. An STD module called `name`.
2. Otherwise a folder `./name`.
3. Otherwise a compile error.

A folder becomes a module, and each `.cm` file in it becomes a sub-module: `geometry.vectors.dot()`. Top-level functions and types are visible to importers by default; struct members follow `public` / `private`.

### Aliases and Macros

```cm
^^use int as num            // type alias
^^define MAX as 100         // text replacement
```

### Conditional Compilation

```cm
^^if OS == WINDOWS
  ...kernel32 calls...
^^else
  ...Linux syscalls...
^^end
```

Built-in names: `OS` (`WINDOWS`, `LINUX`), `ARCH` (`X86_64`), `DEBUG` (true in debug builds).

### Attributes

Attributes use `^^(name)` and apply to the whole file. Names ending in `Def` turn something on, and names ending in `Undef` turn something off.

| Attribute | Effect |
| --- | --- |
| `^^(tfAliasDef)` | `t` and `f` mean `true` and `false` |
| `^^(stringAliasUndef)` | Removes the `string` keyword |
| `^^(hashForDoubleCaretDef)` | `#` may replace `^^` after this line |
| `^^(defaultBitShiftDef)` | Oversized constant shifts mask the count instead of erroring |
| `^^(gotoDef)` | Accepts `goto` as a spelling of `travel` |
| `^^(packStructs)` | Removes struct padding |
| `^^(implementCDef)` | Adds familiar C standard library functions as bare names: malloc, calloc, realloc, free, memcpy, memset, strlen, strcmp, puts, printf and so on. They are Chasm's own versions built on the STD (malloc and free use heap), so no libc is linked |

**Recommended file header order:** attributes, then `^^get`, then `^^define`, then `^^use`.

## Program Entry, Runtime and Inline Assembly

`main` always returns `^int`, takes either no parameters or the argument list, and is implicitly error-returning.

```cm
define ^int main() { ... }
define ^int main(char*[]* args) {      // args.len = argument count
  foreach (string a in *args) { io.println(a); }
  return 0;
}
```

**Ending the program**

- `return 0;` at the end of `main` is optional. If `main` reaches its closing brace, the compiler inserts `return 0;`. Any other exit code, such as `return 1;`, must be written.
- `leaveprog code` exits the program **immediately** from anywhere, even deep inside other functions. It skips every pending `defer`, so prefer `return` from `main` when you can.

```cm
define ^int main() {
  if (!setup()) return 1;     // safer: defers run
  run();
}                             // return 0 inserted here

define ^void fatal(string msg) {
  io.println(msg);
  leaveprog 2                // exits now with code 2, leaveprog requires no semicolon
}
```

### Runtime

The Chasm runtime contains no libc linkage.

| Job | Linux | Windows |
| --- | --- | --- |
| Entry point | `_start`, written in assembly, calls `main` | Entry stub calls `main`, then `ExitProcess` |
| Memory for `heap` | `mmap` / `munmap` syscalls | `VirtualAlloc` / `VirtualFree` |
| Output | `write` syscall | `WriteFile` (`kernel32.dll`) |
| Calling convention | System V AMD64 | Microsoft x64 |

The compiler must follow each platform's calling convention whenever it calls the OS or emits a function other code may call.

### Inline Assembly

```cm
define ^int64 rdtsc() {
  int64 r;
  block asm (clobbers: rdx) {
    rdtsc
    shl rdx, 32
    or  rax, rdx
    mov [r], rax
  };                       // asm blocks end with a semicolon
  return r;
}
```

- Written as `block asm { ... };` in Intel / NASM syntax only, and passed straight to the output.
- Chasm variables are usable by name; the compiler substitutes their addresses.
- The optional `(clobbers: ...)` list names the registers the block changes.

## Standard Library

The STD is C++-sized (containers, algorithms, I/O, OS access) plus JSON, and it is written without libc. `core` is always imported.

| Module | Contents |
| --- | --- |
| `core` (auto) | `Any`, `Equatable`, `Comparable`, `Numeric`, `Allocator`; `Heap`/`heap`, `Arena`, `Pool`, `FixedBuffer`, `DebugAlloc`; `memCopy`, `memSet`, `memCompare`; `bitcast` |
| `io` | `print`, `println`, `readLine`, `File`, buffered readers and writers |
| `fs` | Paths, directories, file info |
| `str` | Growable `String`, formatting, number ↔ text parsing, UTF-8 helpers |
| `math` | Float functions, checked and saturating integer math |
| `collections` | `List<T>`, `Map<K, V>`, `Set<T>`, `Queue<T>`, `Stack<T>`, `LinkedList<T>`, `PriorityQueue<T>` |
| `algo` | `sort`, `binarySearch`, `min`, `max`, `reverse` |
| `cast` | The `cast` function |
| `os` | Environment variables, exit, processes, time and clocks |
| `rand` | Auto-seeded: `randi(int a, int b)`, `randl(long a, long b)`, `randf(float a, float b)`, … |
| `hash` | Hash functions used by `Map` and `Set` |
| `json` | Parse and write JSON |
| `thread` | Threads, mutexes, atomics |
| `net` | Sockets |
| `test` | Unit-test runner used by `cmc` |

**Naming:** the STD follows the idiomatic conventions (see Idiomatic Practices): `camelCase` functions and variables, `PascalCase` structs and errors, `UPPER` enum members.

## The `cmc` Toolchain

`cmc` is one all-in-one tool, like Rust's `cargo`. It compiles, runs, tests, formats, lints and explains errors.

**Building**

`-b` runs the whole pipeline and deletes the intermediate `.asm` and `.o` files, leaving only the executable. Each step also has its own flag, so you can stop after any step or start from its output.

| Flag | Step | Input → output |
| --- | --- | --- |
| `--build` or `-b` | Full build: every step below, then delete intermediates (default) | `.cm` → executable |
| `--build --keep` or `-b --keep` | Full build, keeping the `.asm` and `.o` files | `.cm` → executable + intermediates |
| `--tokens` or `-t` | Lex only; print the token list | `.cm` → terminal |
| `--ast` | Lex and parse; print the syntax tree | `.cm` → terminal |
| `--check` or `-c` | Lex, parse and type-check; report errors, write nothing | `.cm` → terminal |
| `--generate` or `-s` | Generate assembly | `.cm` → `.asm` |
| `--assemble` or `-a` | Assemble with NASM | `.asm` → `.o` |
| `--link` or `-l` | Link with the runtime | `.o` files → executable |
| `--output` or `-o name` | Name the output file of any step |  |

```
cmc -b main.cm -o app           # main.cm → app, nothing else left behind
cmc -s main.cm                  # stop at main.asm
cmc -a main.asm                 # main.asm → main.o
cmc -l main.o util.o -o app     # link object files
```

**Other commands**

| Command | Does |
| --- | --- |
| `cmc --run main.cm` | Build to a temporary executable and run it |
| `cmc --debug` / `cmc --release` | `DEBUG` on (with `DebugAlloc` available) / optimized |
| `cmc --target=windows` / `--target=linux` | Choose the output platform (default is autodetect) |
| `cmc --test` | Run every test in the project |
| `cmc --explain=Error.0042` | Print a longer explanation and example for an error code |
| `cmc --format` | Format every `.cm` file in the project |
| `cmc --format-file='main.cm'` | Format one file |
| `cmc --lint` | Lint every file |
| `cmc --lint-file='main.cm'` | Lint one file |
| `cmc --new=name` | Create a project folder with `main.cm`; the new `main.cm` starts as the hello-world program from the Overview section |

Every compiler error carries a four-digit code from `Error.0000` to `Error.9999`, for example `Error.0001`, so `--explain` can look it up.

### Tests

Tests are `block test("name")` blocks written at the top level of any `.cm` file. `cmc --test` finds and runs every one; normal builds leave them out entirely. The assertions are modeled on GoogleTest.

```cm
block test("add handles negatives") {
  tassert(add(-2, 2) == 0);
  eassert(add(2, 3), 5);
  ltassert(add(1, 1), 3);
  nearassert(0.1 + 0.2, 0.3, 0.0001);
  errassert(open("missing.txt"), FileError.NotFound);
}
```

| Assertion | Passes when |
| --- | --- |
| `tassert(c)` | `c` is true |
| `fassert(c)` | `c` is false |
| `eassert(a, b)` | `a == b` |
| `neassert(a, b)` | `a != b` |
| `ltassert(a, b)` / `leassert(a, b)` | `a < b` / `a <= b` |
| `gtassert(a, b)` / `geassert(a, b)` | `a > b` / `a >= b` |
| `nearassert(a, b, eps)` | Floats differ by at most `eps` |
| `streassert(a, b)` | Two `string`s hold the same characters |
| `errassert(call, E)` | A `!` call fails with error `E` |
| `okassert(call)` | A `!` call succeeds |

Every assertion also has a **non-fatal twin** ending in `expect` instead of `assert`: `texpect`, `fexpect`, `eexpect`, `neexpect`, `ltexpect`, `leexpect`, `gtexpect`, `geexpect`, `nearexpect`, `streexpect`, `errexpect`, `okexpect`. A failed `expect` reports the failure and lets the test keep running, but the test still counts as failed. This matches GoogleTest's `ASSERT_*` versus `EXPECT_*`.

```cm
block test("parse fields") {
  eexpect(rec.id, 7);         // keeps going if wrong
  eexpect(rec.name.len, 5);
  okassert(rec.validate());   // stops here if this fails
}
```

- The first failing assertion stops that test, prints the file, line, test name and both values, then moves to the next test.
- `defer`s inside a test run even when it fails.
- Test names may contain spaces. `test` is only special right after `block`.
- `cmc --test` exits with code 0 when every test passes and 1 otherwise.

## Idiomatic Practices

Idiomatic Chasm keeps every allocation, error and quirk visible right where it happens.

### Memory

1. **Pair `new` with `defer del` on the very next line, in the same block.**

```cm
Node* n = new Node {val: 1};
if (n == NULL) { return 1; }
defer del n;
```

2. **Check `new` for `NULL`** before the `defer`.
3. **Prefer an `Arena` for anything that dies together:** parse trees, per-frame data, per-request data. One `defer del ar;` replaces hundreds of `del`s.
4. **Use a `Pool`** for many same-size objects such as linked-list nodes.
5. **Wrap the allocator in debug builds** and let `DebugAlloc` report leaks at exit:

```cm
^^if DEBUG
  DebugAlloc al = DebugAlloc.wrap(heap);
^^else
  Heap al = heap;
^^end
```

6. **Accept an allocator as a default argument** in any library function that allocates: `Allocator al: heap`.
7. **Use `undefined` only for large buffers you fill immediately.** Everywhere else, rely on zeroing.

### Arrays and Pointers

- Pass arrays as `(arr.ptr, arr.len)` or as a pointer `int32[N]*`; never by value unless they are tiny.
- Write `char*[]` for lists of strings, never `string[]`.
- Prefer `foreach` over index loops when you don't need the index.

### Errors

- Keep `attempt` blocks small: one or two calls, not a whole function.
- Declare variables you need afterward **before** the `attempt` block, initialized to `undefined`.
- Use the narrow form `T!SomeError` on public functions so callers know exactly what to `reject`.
- Give each module its own `error` enum (`FileError`, `NetError`, `JsonError`).

### Operators and Expressions

- Always parenthesize bitwise operations next to comparisons: `(x & 1) == 0`.
- Mark an intended assignment in a condition with extra parentheses: `if ((n = read()))`.
- Prefer `const` over `^^define` for values. When a macro is needed, wrap each parameter and the whole body in parentheses.
- Use `var` when the type is obvious from the right-hand side; write the type when it is not.

### Files and Naming

- File header order: attributes → `^^get` → `^^define` → `^^use`.
- Use `^^get ./name` for your own modules so a future STD module can never replace them.
- Use `bare` only for `io` in small programs.
- Names (idiomatic, not enforced by the compiler): `camelCase` for variables and functions, `PascalCase` for structs, interfaces, error types and error members (`FileError.NotFound`), `UPPER` for enum members (`Level.LOW`), `^^define` macros and `const` values.
- Most attribute names end in `Def` (turn on) or `Undef` (turn off).

## Quirks

These are deliberate Chasm behaviors that will surprise programmers coming from C, C++ or other languages.

### Types and Declarations

| Quirk | Example | Notes |
| --- | --- | --- |
| `short` is 8 bits | `short s = 200;` wraps to −56 | C's `short` is at least 16 bits |
| `long` is always 64 bits | `long x;` | On Windows, C's `long` is 32 bits |
| `float` is 64 bits | `float x = 0.1;` | C's `float` is 32 bits |
| `string[]` is a char buffer | `string[] b = "hi";` is `char[3]` | Reads like an array of strings |
| `string` can be removed | `^^(stringAliasUndef)` | Code using `string` breaks in that file |
| `t` / `f` become reserved | `^^(tfAliasDef)` | `int t = 0;` stops compiling |
| Both declarators are pointers | `int* a, b;` | In C, `b` is a plain `int` |
| `pszt` vs `szt` | 4 vs 8 bytes on a 32-bit target | - |
| Enum members repeat the name | `Level.LOW` inside `enum Level` | Most languages write bare `LOW` there |
| No `f` suffix on hex | `0x1f` is 31 | Expected to make a float |
| Leading zero is decimal | `010` is 10 | In C it is 8 (octal) |
| `float` and `double` never mix | `double64 d = f;` needs `(double64)f` | `float64` and `double64` have identical bits |

### Operators

| Quirk | Example | Notes |
| --- | --- | --- |
| Ternary uses `=>` | `a > b => a : b` | Every other language writes `?` |
| Postfix only | `++x` is an error | Allowed in C |
| Assignment in conditions | `if (x = 5)` is always true | A classic typo for `==` that Chasm allows |
| C precedence kept | `x & 1 == 0` is `x & (1 == 0)` | Looks like it tests the low bit |
| Integer narrowing is silent | `int8 b = 300;` gives 44 | But float → int needs a cast |
| Signed/unsigned mixing is an error | `-1 < 1u` won't compile | C compiles it, and gets `false` |
| Oversized constant shifts error | `x << 64` | C compiles it as undefined behavior |
| `&=` has two meanings | `struct S &= Shape` vs `x &= 3` | Bitwise AND-assign elsewhere |

### Control Flow

| Quirk | Example | Why it surprises |
| --- | --- | --- |
| `break` leaves **all** loops | `break;` inside nested loops | In C it leaves one loop |
| `stop` leaves one loop | `stop;` | This is C's `break` |
| `goto` is `travel` | `travel retry;` | Unless `^^(legacyGotoDef)` |
| No switch fallthrough | `is 1: {...} is 2: {...}` | C falls through without `break` |
| `else` is `other` | `} other {` | - |
| Bare blocks need `block` | `block { ... }` | A bare `{` is a syntax error |
| `asm` blocks end with `;` | `block asm { ... };` | Other blocks don't |
| `leaveprog` skips defers | `leaveprog 1` | Your cleanup code never runs; return 1 from main is safer |
| main returns 0 by itself | No return at the end of main | Every other ^int function must return a value |

### Memory, Errors and Modules

| Quirk | Example | Why it surprises |
| --- | --- | --- |
| Arrays never decay | `sizeof(a)` is the whole array, even in callees | Large arrays are copied when passed |
| No bounds checks, even in debug | `a[10]` on `int[4]` | Silent memory corruption |
| `del` on an arena frees everything | `del ar;` | `del` on an object frees one thing |
| `del` inside an arena does nothing | `del n;` where `n` came from `ar` | Memory returns only with the arena |
| `defer` is block-scoped | `defer` inside `if { }` runs at that `}` | Go's `defer` waits for function exit |
| `attempt` scopes variables | `f` is gone after `attempt { File f = ...; }` | Must declare it before |
| `main` is secretly `!` | An uncaught error exits with code 1 | Other functions must declare `!` |
| Lambda captures copy | `\|x\|` takes `x`'s value at creation | Later changes to `x` aren't seen |
| Import lookup prefers the STD | `^^get math` | A local `./math` folder is ignored when the STD has one |
| Multi-line strings | A missing `"` swallows code until the next `"` | An unclosed string is reported at its opening `"`, but a later stray `"` can make it look closed |
| `^^define` pastes text | `SQ(1+2)` is 5 | Typed `const` doesn't have this problem |
| Not every attribute ends in `Def` / `Undef` | `^^(packStructs)` | Exceptions to the naming idiom |
