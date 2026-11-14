---
format_id: ini-syntax
title: INI syntax
summary: Defines the section, key, value, and comment syntax accepted by the OpenTS INI parser.
kind: syntax
applies_to:
  - OpenTS INI files
source_files:
  - code/ini.cpp
  - code/ccini.cpp
---

A section begins with `[`, ends at the first `]`, and is named by the text between them. An assignment uses the first `=` to separate its key from its value. Whitespace around the key and around the value is removed.

```ini title="example.ini"
[General]
Name=Example ; text after the semicolon is a comment
```

Section and key names are case-sensitive: `[GAPOWR]` and `[gapowr]` are different sections, and `Strength=` does not set `strength`.

A semicolon starts a comment that runs to the end of the line, wherever it appears, so a value cannot contain one.

These lines are ignored:

- lines before the first section;
- lines without `=`;
- assignments with an empty key or an empty value.

An empty value therefore cannot clear a value that an earlier file set.

Text is UTF-8, and a byte order mark at the start of a file is ignored. A line that is not valid UTF-8 is read as Windows-1252, so a file written in that code page keeps its accented characters. When the game saves an INI file, it writes UTF-8 without a byte order mark.

A line may be any length. When a setting stores its value in fixed-size storage, the value is cut to fit, and the debug log records the cut once per key.

## Line breaks in text

An at sign (`@`) starts a new line in the text of a sidebar or map tooltip, a mission's written briefing, and the map selection screens. Write `\@` to print an at sign there, and `\\` to print a backslash. Any other backslash prints as written. For example, `Drop at LZ\@2@Hold` prints `Drop at LZ@2` and then `Hold` on the next line, and `C:\\@Next` prints `C:\` and then `Next`.

The rule covers everything those places print, such as a unit's name in its tooltip. The same text shown anywhere else, such as a name under a cameo, prints `@` and `\` as written.

## Repeats and later files

A repeated section header continues the section already read, and a repeated key replaces the value read earlier. The same happens when a file is loaded on top of an earlier one. A section the earlier file lacks is added, and an existing section gains or replaces keys. With Firestorm enabled, for example, `ARTFS.INI` is loaded on top of `ART.INI`.

A replaced key moves to the end of its section. This changes its position in a section read in order, such as a type list.

A repeat within one file is written to the debug log with the file, section and key. The log does not show a value that a later file replaces, so check the later files to find which one set a value.

## Malformed values

What happens to a value the game cannot read depends on the kind of value its key expects.

| Kind of value | How it is read | When it cannot be read |
| --- | --- | --- |
| Yes or no | By its first character, in either case. `Y`, `T` or `1` means yes; `N`, `F` or `0` means no. `On` is not recognized. | The setting keeps its default. |
| Whole number | From its leading digits, so `12abc` reads as `12`. A value that starts with `$` or ends in `h`, in either case, is read as hexadecimal instead, so `$10` and `10h` both read as `16`. | The value reads as `0`. A value read as hexadecimal keeps the default when no hexadecimal digit starts the number, as in `$G` or `high`. |
| Decimal number | From the start of the value, so `1.5x` reads as `1.5`. A percent sign anywhere in the value divides the number by 100, so `50%` reads as `0.5`. | The setting keeps its default. |
| Point, offset, vector, color or rectangle | As two, three or four comma-separated numbers, as the key requires. Spaces around the commas are allowed, and anything after the last number is ignored. | The setting keeps its default; no part of the value is kept. |

A decimal number cannot be read when the value does not start with a number. A group of numbers cannot be read when it has fewer numbers than the key requires, or something other than a number where one is expected. For these two kinds, the debug log records the file, section, key and value.

An omitted key is not malformed: it reads as its default.
