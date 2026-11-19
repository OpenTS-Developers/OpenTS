---
title: Take an animation's frame counts from the shape it draws
category: fix
release: 0.3.0
targets:
- type: key
  id: End
  effect: changed
- type: key
  id: LoopEnd
  effect: changed
- type: key
  id: Image
  scope: animtype
  effect: changed
- type: key
  id: Theater
  scope: animtype
  effect: changed
- type: key
  id: NewTheater
  scope: animtype
  effect: changed
breaking: true
migration:
- On a looping animation that sets `End` but not `LoopEnd`, set `LoopEnd` to the shape's frame count to keep the middle passes running over the whole shape.
- On a `Reverse=yes` animation that sets `End` but not `LoopEnd`, set `LoopEnd` to the shape's frame count to keep the frame it opens on.
- On the `TurretAnim` of a `Charges=yes` structure that sets `End` but not `LoopEnd`, set `LoopEnd` to the shape's frame count to keep the time the structure takes to charge.
- On an animation that sets `End=0` or `LoopEnd=0` to end a pass at its first frame advance, set it to `1`.
credit:
- ZivDero
---

An unset, `0` or `-1` `End` now plays the frame count of the shape the animation draws at that moment. Before, an unset `End` kept the first frame count measured while the rules were read, which could come from `<AnimType ID>.SHP` rather than the shape the animation drew. An `End=0` or `LoopEnd=0` stayed zero unless the animation set `DemandLoad=yes`, or `NewTheater=yes` without `Theater=yes`; those took their counts from the shape as if the key were unset.

An unset, `0` or `-1` `LoopEnd` now always takes the `End` count. `End=` alone therefore also ends the middle passes of a looping animation, sets the frame a `Reverse=yes` animation opens on, and sets how long a `Charges=yes` structure's `TurretAnim` plays before the structure is charged. Before, an unset `LoopEnd` took the shape's whole frame count on an animation that was not demand-loaded, even when `End` was set.

An animation now draws the same file in every scenario and after a saved game is loaded, chosen in the order the [`Image`](/keys/image/#scope-animtype), [`Theater`](/keys/theater/#scope-animtype) and [`NewTheater`](/keys/newtheater/#scope-animtype) pages give. Before, an animation that set neither `Theater=yes` nor `NewTheater=yes` drew nothing after a saved game was loaded when `<Image ID>.SHP` was missing, even when `<AnimType ID>.SHP` existed. A `Theater=yes` animation in a scenario that repeated the previous scenario's theater tried the theater's file under its Image ID before `<AnimType ID>.SHP`. A `NewTheater=yes` animation whose renamed file was missing drew nothing after a saved game was loaded, even when it drew the unrenamed file before the save. The largest frame, on which `Crater`, `Scorch` and `Flamer` act, comes from the same shape, and is frame 0 when the animation has none.

An `End=-1` animation whose shape cannot be found no longer crashes the game when it is created, when it advances a frame, or when another animation changes into it through `Next=`. It draws nothing, and its last pass ends at its first frame advance.
