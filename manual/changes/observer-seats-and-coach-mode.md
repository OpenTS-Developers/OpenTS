---
title: Watch a multiplayer match as an observer
category: feature
release: 0.2.0
targets:
- type: system
  id: observers
  effect: added
- type: format
  id: spawn-ini
  effect: changed
- type: format
  id: save-games
  effect: changed
- type: system
  id: map-visibility
  effect: changed
- type: system
  id: cloaking
  effect: changed
- type: system
  id: sidebar
  effect: changed
- type: system
  id: veterancy
  effect: changed
- type: system
  id: power
  effect: changed
- type: system
  id: ion-storms
  effect: changed
credit:
- ZivDero
- Iran
- dkeeton
---

A launch file may seat people who watch the match rather than play it. An observer's house starts defeated with nothing on the map, and is given the whole map with the radar up; it sees hidden objects and decorations as their owners do. It shows the match time where its credits would be, and can message only everyone or the other observers. It is left out of the score screen, the name list and the statistics report.

`CoachMode` in the launch file's `[Settings]` section is honored: a defeated player keeps allied vision and private chat instead of the whole map. Without it a defeated player now has the fog lifted, the regrowth stopped, and hidden objects and decorations shown, as an observer does.

Iran wrote the spawner's observer seats, from the code he wrote for Red Alert. dkeeton added coach mode to ts-patches.
