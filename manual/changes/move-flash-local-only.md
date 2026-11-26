---
title: Keep the move order marker out of the game in every kind of game
category: fix
release: 0.3.0
targets:
- type: key
  id: MoveFlash
  effect: changed
- type: key
  id: YSortAdjust
  effect: changed
credit:
- ZivDero
---

In a campaign or skirmish game, the marker at a move order's destination was an ordinary animation: each move order shifted the numbering of every object created after it, and random settings on the type drew from the game's random numbers. In a network game, each move order set the marker type's `YSortAdjust` to `-5000` on the ordering player's machine, and random settings on the type could put the game out of sync.

The marker now plays only on the ordering player's machine in every kind of game and is no longer a game object. It no longer has any of an animation's game effects, such as damage, scorch marks or spawned animations. A ground-layer marker is drawn behind the objects in its layer, and the type's `YSortAdjust` is left alone.

A move order no longer crashes the game when `MoveFlash` is unset or set to `none`; it shows no marker.
