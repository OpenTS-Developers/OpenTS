---
title: Report a completed save in the message list
category: feature
release: 0.2.0
targets:
- type: format
  id: save-games
  effect: changed
- type: command
  id: QuickSave
  effect: changed
credit:
- ZivDero
- Rampastring
---

A save the player asks for now reports `Game saved.` in the message list once the file is written, in place of the box the save dialog used to show; a save from the options menu of a game against other machines reported nothing at all. The line is posted at the frame boundary rather than where the save ran, so a save made from the menus is reported when the player returns to the map, and two players saving on one frame share one save and one line.

An automatic save turns its own `Auto-saving...` line into `Game auto-saved.` instead of adding a second one, and `Auto-save failed.` became `The game could not be auto-saved.` so the four lines a save can leave read alike. The random map generator keeps its own box.
