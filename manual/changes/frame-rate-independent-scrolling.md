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
credit: [ZivDero]
---

Edge scrolling and right-button coasting now move the view by elapsed time instead of once per
drawn frame. A machine drawing several hundred frames a second used to throw the view across the
map as soon as the pointer reached an edge.

A step from the scroll table now lands thirty times a second, twice the original game's rate. The
table, the speed the ramp climbs at, and `ScrollMultiplier` are unchanged, so the scroll speed
options keep their order and their relative spacing.

Coast methods `1` and `2` move by the distance the hand covered since the last reading, and are
unchanged. A `ScrollMethod` outside `0` through `2` now scrolls nothing.
