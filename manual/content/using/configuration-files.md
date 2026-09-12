---
title: Configuration files
summary: OpenTS reads player options, hotkeys, rules, art, media registries, and scenario data from separate files.
category: configuration
source_files:
  - code/init.cpp
  - code/options.cpp
  - code/sun.h
related:
  - type: using
    id: game-data
---

Configuration is documented in five areas rather than one. The table names what each area covers, so a reader chasing a single fact can go straight to the area that owns it:

| Area | Covers |
| --- | --- |
| [INI Reference](/reference/) | Accepted keys, sections, value types, and established omission behavior |
| [Formats](/formats/) | File loading, registration sections, record structure, and companion files |
| [Commands](/commands/) | Hotkey command names and fixed controls |
| [Command line options](/using/command-line/) | Options accepted on the OpenTS command line |
| [Mapping](/mapping/) | Scenario sections, triggers, TeamTypes, TaskForces, Scripts, and AI triggers |

`SUN.INI` stores local player options. `KEYBOARD.INI` maps command names to keys. [`UI.INI`](/formats/ui-ini/), which a mod or deployment may ship, styles the order lines. Rules, art, sound, theme, and scenario files supply game and mod data. A deployment that keeps these under other names writes them in its [`OPENTS.INI`](/formats/opents-ini/#the-files-it-reads).

The engine loads its configuration files in a defined order. Use the relevant Format page when file selection, layering, registration, or section identity affects the result.
