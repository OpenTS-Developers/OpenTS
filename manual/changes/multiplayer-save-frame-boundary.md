---
title: Save multiplayer games after the frame is retired
category: fix
release: 0.1.0
targets:
- type: format
  id: save-games
  effect: changed
credit: [ZivDero]
---

A synchronized multiplayer save command now records one pending request instead of writing the file while game commands are still executing. Repeated commands before the end of the frame are coalesced, and the save is written after objects marked for deletion have been retired. A player leaving permanently disables saving for that match and cancels any pending request. The Save button stays disabled through a mission restart, and only a newly selected game restores it.
