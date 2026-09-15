---
format_id: theme-ini
title: THEME.INI
summary: Registers music theme IDs and their playlist metadata.
kind: file
filenames:
  - THEME.INI
  - THEME01.INI
key_scopes:
  - file: theme01.ini
    section:
      kind: identifier
      source: theme
related:
  - { type: format, id: aud }
  - { type: format, id: opents-ini }
source_files:
  - code/init.cpp
  - code/theme.cpp
---

Startup reads `THEME.INI` and `THEME01.INI` into one database and builds the theme list from it. Whichever files are present are read, the expansion's file over the base one. Either file alone is enough, and when neither can be read, startup reports the failure and exits. Whether Firestorm is installed does not decide which file is read.

`[Themes]` values register theme IDs. The text to the left of the `=` is only a label, and the entries keep the order they are read in. Each ID names a section and an `.AUD` music file with the same base name. A later definition updates an existing ID instead of creating a duplicate.

```ini title="THEME.INI"
[Themes]
0=MYTHEME

[MYTHEME]
Name=Example theme
Length=3.27
Normal=yes
```
