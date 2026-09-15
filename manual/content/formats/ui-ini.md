---
format_id: ui-ini
title: UI.INI
summary: Sets how a selected object's order lines and a firing vehicle's sighting laser are drawn.
kind: file
source_files:
  - code/uicontrol.cpp
  - code/init.cpp
filenames:
  - UI.INI
key_scopes:
  - file: ui.ini
    section:
      kind: literal
      name: Ingame
related:
  - type: system
    id: action-lines
  - type: format
    id: opents-ini
  - type: using
    id: configuration-files
---

The file is optional and every key has a default, so a file names only what it changes. It holds one section, `[Ingame]`.

```ini title="UI.INI"
[Ingame]
AlwaysShowActionLines=yes
MovementLineDashed=yes
MovementLineColor=0,255,0
NavComQueueLineThick=yes
```

A color is three values from 0 to 255 for red, green and blue, separated by commas; a value that is not three numbers keeps the default. [Action lines](/systems/action-lines/) explains what each line is and when it is drawn.

## When the file is read

The file is read once at start-up, after the game archives are registered. It is read again each time a side's archives are mounted, which happens as a scenario loads, so a copy inside a side's archive takes effect for that side. Each read resets every value to its default first, so a value the file no longer names goes back to the default instead of lingering from an earlier read.

The file is opened through the game's file layer, so it may be loose in any folder the game searches or inside an archive. A loose copy is used ahead of an archived one. [OPENTS.INI](/formats/opents-ini/#the-order-files-are-searched-for-in) lists the folders and the order they are searched in.
