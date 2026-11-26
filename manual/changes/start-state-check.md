---
title: Stop a network game whose machines loaded different games
category: fix
release: 0.3.0
targets:
- type: system
  id: out-of-sync-recovery
  effect: changed
credit:
- ZivDero
---

A new network game compared nothing at its start, so machines that loaded different scenarios, rules or object types started the game together and went out of sync later. Each machine now computes a checksum of the game it loaded just before the first frame, and a game whose machines disagree stops at its start with `Scenarios don't match.` Each machine's debug log lists its checksum for each part of the game, so the part that differs can be found. A resumed saved game is checked as before.
