---
title: Name the difficulty a campaign mission is played at
category: feature
release: 0.2.0
targets:
- type: system
  id: difficulty
  effect: changed
- type: format
  id: spawn-ini
  effect: changed
credit:
- ZivDero
- Rampastring
- CCHyper
- dkeeton
---

A campaign mission now names the difficulty it is played at in a message as it starts. The name says how hard the mission is, so it mirrors the section the computer houses read: `[Easy]` is announced as Hard, `[Normal]` as Medium and `[Difficult]` as Easy.

`DifficultyName` in a launch file replaces that name, which is how a client offering more difficulties than the game's three names the one it chose.
