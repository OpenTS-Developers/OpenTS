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
filenames:
- TUTORIAL.INI
---

`[Tutorial]` keys are the numbers [Text Trigger...](/mapping/actions/taction-text-trigger/) names,
and each value is the line that trigger prints. The file is read once as the game starts; a missing
one leaves no lines and is not reported.

A scenario may carry a `[Tutorial]` section of its own. Its lines stand in front of the file's for
as long as that scenario is played: a number the file carries takes the scenario's line instead, a
number it does not carry is added, and the file itself is left alone for the next mission. The
section is read from the scenario file, the one carrying the mission's `[Actions]`. The companion
`.INI` a campaign mission may ship beside its map, and the rules files, are not consulted for it.

```ini title="TUTORIAL.INI"
[Tutorial]
120=Hold the ridge until the transports arrive.
```

```ini title="map file"
[Tutorial]
120=Hold the bridge instead.
200=A line of this mission's own.
```

A key has to be a whole number and nothing more. One with a tail is skipped and written to the
debug log rather than read as the number it starts with, and two spellings of one number, such as
`120` and `0120`, are the same line with the later value winning. A key with nothing after the `=`
never becomes an entry, so a scenario can replace a line with different text but never with none.

The file ships translated; an override does not go through any translation of its own, so a
scenario replaces a line in whatever language it was written in for every player who runs it.

A saved game carries the scenario's lines, since a load never re-reads the map. The file's own lines
are not saved and are read again as the game starts.
