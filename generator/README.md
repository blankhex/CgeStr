# Generator

This utility should be used to regenerate tables based on Unicode Character
Database (UCD).

Current version of this utility builds tables and functions for the following
properties:

- Case mappings for lower, upper, title cases (1:1 and 1:M)
- Case folding (1:1 and 1:M)
- General category

## Usage

Compile `Tables.c`

```
gcc Tables.c -o Tables
```

Download and put `UnicodeData.txt`, `CaseFolding.txt`, `SpecialCasing.txt` near
the compiled `Tables` program, run it and pipe output into `UCD.c`.

```
Tables > ../UCD.c
```

You can download required files from [here](https://www.unicode.org/Public/UCD/latest/ucd)
