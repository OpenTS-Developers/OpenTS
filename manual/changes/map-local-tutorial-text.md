---
title: Read a map's own tutorial lines
category: feature
release: 0.2.0
targets:
- type: format
  id: tutorial-ini
  effect: added
- type: action
  id: TACTION_TEXT_TRIGGER
  effect: changed
- type: format
  id: save-games
  effect: changed
credit:
- ZivDero
- CCHyper
---

A scenario can carry a `[Tutorial]` section of its own, replacing a line `TUTORIAL.INI` supplies or
adding one for as long as that mission is played, so mission text no longer has to go into the one
file every player shares. Those lines travel with the mission's saved game, since a load never
re-reads the map. A line is no longer cut at 299 characters, and a key that is not a whole number is
skipped and logged rather than read as line 0.

Saves from earlier development snapshots of this cycle no longer load, because the loose values grew
by the scenario's lines.

CCHyper is credited for the Vinifera feature this ports.
