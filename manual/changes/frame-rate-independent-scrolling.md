---
title: Scroll the tactical map at one speed on every machine
category: fix
release: 0.2.0
targets:
- type: key
  id: ScrollRate
  effect: changed
- type: key
  id: ScrollMethod
  effect: changed
- type: key
  id: ScrollMultiplier
  effect: changed
credit: [ZivDero, FunkyFr3sh]
---

Edge scrolling and right-button coasting now move the view by elapsed time. They used to move it once per drawn frame, so a machine drawing several hundred frames a second threw the view across the map as soon as the pointer reached an edge.

A step from the scroll table lands sixty times a second, four times the original game's rate. The table is unchanged, and so are the speed the ramp climbs at and `ScrollMultiplier`. The scroll speed options therefore keep their order and their relative spacing. Coast methods `1` and `2` are unchanged, and move by the distance the hand covered since the last reading. A `ScrollMethod` outside `0` through `2` now scrolls nothing.

FunkyFr3sh is credited for the ts-patches scroll rate limiter, which holds the same two paths against the same fault by a different means.
