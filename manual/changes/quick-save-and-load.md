---
title: Save and load a quick save from the keyboard
category: feature
release: 0.2.0
targets:
- type: command
  id: QuickSave
  effect: added
- type: command
  id: QuickLoad
  effect: added
- type: format
  id: save-games
  effect: changed
credit: [ZivDero]
---

Two commands save and load a campaign or skirmish game without the options menu. Quick Save writes `QUICKSAVE.SAV` in a campaign and `QUICKSAVE_SKIRMISH.SAV` in a skirmish, behind the saving box a menu save shows, and reports the result in the message list. Quick Load restores the file for the kind of game being played, or says there is none. Both arrive unbound. Neither acts in a game against other machines, while input is locked, or once the game is being won or lost.
