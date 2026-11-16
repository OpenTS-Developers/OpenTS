---
title: Refuse a save with an unmatched object reference instead of exiting
category: fix
release: 0.3.0
targets:
- type: format
  id: save-games
  effect: changed
credit:
- ZivDero
---

A save whose objects all load but which refers to an object it does not contain used to show `Save game load failed!` and close the game. The load now fails like any other damaged save: the load dialog and quick load show `Error loading game!`, a multiplayer load leaves the match, and a resume from a launch file is refused.

After a load that failed partway, a save from the options menu used to write the partly loaded game. After any failed load, that save now shows `Error saving mission!` and writes nothing until a game is loaded or started, as quick saves and autosaves already did.
