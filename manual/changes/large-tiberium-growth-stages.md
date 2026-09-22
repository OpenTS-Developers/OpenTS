---
title: Give every Tiberium set twelve growth stages
category: fix
release: 0.2.0
targets:
- type: key
  id: Image
  scope: tiberium
  effect: changed
credit:
- ZivDero
- Rampastring
- dkeeton
---

A Tiberium type on `Image=2` now has the same twelve growth stages as every other set, instead of one. Cells of that set take a stage from the smoothing pass, gain stages from growth, and can be created by spreading, none of which a single stage allowed.

The shipped rules are unmoved by it: `Cruentus` is the only stock type on that set, and it holds `GrowthPercentage=0` and `SpreadPercentage=0`, so it still neither grows nor spreads. A mod that wants the set harvested also has to give its overlays a land type that harvesting reads, because the shipped ones are rock.
