---
title: Do not repeat a score the game does not have
category: fix
release: 0.2.0
targets:
- type: key
  id: Repeat
  effect: changed
credit: [ZivDero]
---

A score marked to repeat was handed back by the playlist forever when its audio file was missing from the mixfiles: nothing else was ever picked, and the game played no music at all until a track was chosen by hand. Such a score is no longer offered. Starting a score that will not play no longer records it as the one playing either, where before a score that never started could not be stopped, so it stayed current and was tried again on every frame.
