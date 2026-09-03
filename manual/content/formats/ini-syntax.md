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

A section begins with `[`, ends at the first `]`, and supplies the section name between them. An assignment uses the first `=` to separate its key from its value. Surrounding whitespace is removed from both parts.

```ini title="example.ini"
[General]
Name=Example ; text after the semicolon is a comment
```

A line beginning with `;` is a comment. A semicolon also starts an inline comment. Lines before the first valid section and assignments with an empty key or value are ignored.

Loading a second file into a database that already holds sections merges the two rather than replacing what is there: a section the database does not carry is added whole, and an assignment repeating a key already present overwrites that key's value.

A file that repeats a section header continues the section it already opened. An assignment repeating a key overwrites the value read earlier and moves the key to the end of its section. A file is therefore read the same way whether it opens a database or merges into one that already holds sections. A repeat within one file is written to the debug log with the file, section and key; a later file overriding an earlier one is not, since that is how the rules files stack.

## Malformed values

A written value is converted by the reader for the kind of value expected. A value the reader cannot convert leaves the setting at its default, and the debug log records the file, section, key and value.

A floating-point number is read from the start of the value, so a value that does not start with a number is malformed. A percent sign anywhere in the value divides the number by 100 after it is read, so `50%` reads as `0.5`.

A point, offset, vector, color or rectangle is a comma-separated list of numbers, two, three or four of them as the key requires. Spaces around the commas are allowed, and anything after the last number is ignored. A value with fewer numbers than the key requires, or with something other than a number where one is expected, is malformed as a whole: no part of it is kept.

An omitted key is never malformed. It reads as its default without a log line.
