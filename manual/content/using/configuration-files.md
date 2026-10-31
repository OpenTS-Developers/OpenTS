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

Look up a configuration fact in the area that covers it:

| Area | Covers |
| --- | --- |
| [INI Reference](/reference/) | Accepted keys, sections, value types, and established omission behavior |
| [Formats](/formats/) | File loading, registration sections, record structure, and companion files |
| [Commands](/commands/) | Hotkey command names and fixed controls |
| [Command line options](/using/command-line/) | Options accepted on the OpenTS command line |
| [Mapping](/mapping/) | Scenario sections, triggers, TeamTypes, TaskForces, Scripts, and AI triggers |

The game reads its configuration from these files:

- `SUN.INI` stores the player's options.
- [`KEYBOARD.INI`](/formats/keyboard-ini/) assigns keys to command names.
- [`UI.INI`](/formats/ui-ini/), which a mod or deployment may ship, sets how order lines, the sighting laser, control group numbers and health bars are drawn.
- Rules, art, sound, theme, and scenario files supply game and mod data.

A deployment can rename these files in [`OPENTS.INI`](/formats/opents-ini/#the-files-it-reads): the settings file, `UI.INI`, the tutorial file, and the rules, language rules, multiplayer rules, art, AI, sound, theme, and campaign list files together with their expansion copies. `KEYBOARD.INI` and scenario files cannot be renamed.

The game looks for each file in the [search order](/formats/opents-ini/#the-order-files-are-searched-for-in) and uses the first copy it finds. Settings and hotkeys the player saves are written to the first directory in that order, so the saved copy is read ahead of a shipped one.
