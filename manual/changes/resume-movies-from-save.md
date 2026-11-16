---
title: Take a resumed game's movie setting from its save
category: fix
release: 0.3.0
targets:
- type: format
  id: spawn-ini
  effect: changed
credit:
- ZivDero
---

A game resumed from `SPAWN.INI` with `LoadSaveGame=yes` used to take `PlayMoviesInMultiplayer` from that file. The setting is now kept in the save, and a resumed game plays movies or leaves them out as the saved game did.
