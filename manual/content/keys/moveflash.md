---
key: MoveFlash
summary: The animation dropped on the cell a move order points at.
see_also: [YSortAdjust]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
MoveFlash=MOVEFLSH ; an AnimType registered in [Animations]
```

When the player orders a move, the `MoveFlash` animation plays at the cell the player clicked. It sits at ground height there, or on the bridge deck if the cell has a bridge. A move order for a group plays it at most once, not once per object. If no rules file sets `MoveFlash`, or it is set to `none`, a move order shows no marker.

In every kind of game, the marker plays only on the machine of the player who gave the order. Saves, the multiplayer synchronization check, and the other players never see it, and its random settings, such as [`RandomRate`](/keys/randomrate/) and [`RandomLoopDelay`](/keys/randomloopdelay/), use a random generator that the game's outcome does not depend on.

The marker plays its frames, its loops, the animations in its [`Next`](/keys/next/) chain, and its `Report` sound. It has none of an animation's game effects, such as [`Damage`](/keys/damage/), a [`Scorch`](/keys/scorch/) mark, [`Spawns`](/keys/spawns/), a [`TrailerAnim`](/keys/traileranim/), or the light of an [`AlphaImage`](/keys/alphaimage/).

A marker in the ground layer is drawn behind every object in that layer, and any other marker is drawn after every object in its layer, so the type's [`YSortAdjust`](/keys/ysortadjust/) does not move it.
