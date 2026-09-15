---
key: AutoSaveInterval
summary: How many frames pass between one automatic save and the next in a game started from the menu, or zero for none.
see_also: [GameSpeed]
when_omitted:
  kind: value
  value: "10800"
---

When the frames have run out, the game posts `Auto-saving...` to the message list and writes the save at the next frame boundary. That line is then replaced with the outcome. [Save games](/formats/save-games/#automatic-saves) owns the names the saves take, how they rotate, and what starts the count over. Frames are the game's own clock, so a faster [`GameSpeed`](/keys/gamespeed/) saves more often by the wall clock. A game against other machines arranged from the menu never saves automatically, whatever the figure, because every machine would have to hold the same one.

A game that a [launch file](/formats/spawn-ini/#automatic-saves) starts takes the file's interval instead of this one, and a file that names none turns the saves off.

The figure is read from `sun.ini` when the game starts and written back with the other options; no dialog offers it.
