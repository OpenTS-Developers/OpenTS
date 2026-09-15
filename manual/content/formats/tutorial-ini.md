---
format_id: tutorial-ini
title: TUTORIAL.INI
summary: Holds the numbered lines a text trigger prints, which a scenario may replace or add to.
kind: file
source_files:
- code/tutorial.cpp
- code/init.cpp
- code/scenario.cpp
- code/taction.cpp
related:
- type: action
  id: TACTION_TEXT_TRIGGER
- type: key
  id: MessageDelay
- type: format
  id: opents-ini
filenames:
- TUTORIAL.INI
---

`[Tutorial]` keys are the numbers [Text Trigger...](/mapping/actions/taction-text-trigger/) names, and each value is the line that trigger prints. The file is read once as the game starts, and a missing one leaves no lines and is not reported.

A scenario may contain a `[Tutorial]` section of its own. Its lines stand in front of the file's for as long as that scenario is played. Where both have the same number, the scenario's line is the one printed. A number the file does not have is added, and the file itself is left alone for the next mission. The section is read from the scenario file, the one with the mission's `[Actions]`. The companion `.INI` a campaign mission may ship beside its map, and the rules files, are not read for it.

```ini title="TUTORIAL.INI"
[Tutorial]
120=Hold the ridge until the transports arrive.
```

```ini title="map file"
[Tutorial]
120=Hold the bridge instead.
200=A line of this mission's own.
```

A key has to be a whole number and nothing more. One with a tail is skipped and written to the debug log, rather than read as the number it starts with. Two spellings of one number, such as `120` and `0120`, are the same line, and the later value wins. A key with nothing after the `=` never becomes an entry, so a scenario can replace a line with different text but never with none.

Neither the file's lines nor a scenario's are looked up in a translation table, so a scenario replaces a line in whatever language it was written in, for every player who runs it.

A saved game holds the scenario's lines, since a load never re-reads the map. The file's own lines are not saved and are read again as the game starts.
