# CgeStr - Unicode String Library for C

A lightweight, dependency-free C library for UTF-8 string processing with full
Unicode support.

## Features

- UTF-8 encoding and decoding
- UTF-16 encoding and decoding
- Unicode case mapping: lowercase, uppercase, titlecase, case folding
- Full case mapping functions returning multiple runes when needed
- Character classification: isControl, isDigit, isLetter, isSpace, and others
- Unicode category lookup via `CgeRuneCategory`
- Case-sensitive and case-insensitive string comparison
- Substring and rune search
- Prefix and suffix checking
- String trimming (left, right, both)
- String splitting by rune
- Iteration over Unicode code points using callback interface

## Build Systems

- CMake - supports Linux, macOS, Windows (MSVC, MinGW)
- Makefile.posix - for GCC/Clang on POSIX systems
- Makefile.mingw - for MinGW on Windows
- Makefile.win32 - for MSVC with NMake

Builds a static library. No shared library or external dependencies.

## Usage Example

```c
#include "CgeStr.h"
#include <stdio.h>

void print_rune(uint32_t rune, void *user) {
    printf("U+%04X ", rune);
}

int main() {
    CgeStr str = CGE_STR_LIT("Héllo, 世界!");
    CgeStrIter(str, print_rune, NULL);
    printf("\n");
    return 0;
}
```

## Portability

- Written in C89+stdint.h
- No dynamic memory allocation
- No external dependencies

## License

0BSD - a permissive license with no attribution required.
