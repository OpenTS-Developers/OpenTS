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

A score marked to repeat is no longer handed back by the playlist when its audio file is
missing from the mixfiles. Before, such a score was answered with forever: nothing else was
ever picked, and the game played no music at all until a track was chosen by hand.

Starting a score that will not play no longer records it as the one playing either. A score
that never started could not be stopped, so it stayed current and was tried again on every
frame.

Together these matter to any installation that ships a reduced set of scores. A deployment
whose launcher plays its own menu music, and which therefore leaves the game's menu and map
selection tracks out, would reach the first mission with its music already wedged on a track
that does not exist.
