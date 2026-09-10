---
title: Name the deployment's own files in OPENTS.INI
category: feature
release: 0.2.0
targets:
- type: format
  id: opents-ini
  effect: changed
- type: format
  id: sound-ini
  effect: changed
- type: format
  id: theme-ini
  effect: changed
- type: format
  id: tutorial-ini
  effect: changed
- type: format
  id: ui-ini
  effect: changed
credit:
- ZivDero
---

A deployment names its own game data files in `OPENTS.INI`: rules, artwork, AI, sound,
music, campaigns, translated rules, tutorial text and interface, the expansion copy of each,
the file a player's settings are written back to, and the two palettes the game starts with.
A name it does not write keeps the one Tiberian Sun uses. The expansion rules file it names
is also what the game looks for to decide the expansion is installed.

A palette file the game cannot find leaves that palette unchanged. It stopped the game
before.
