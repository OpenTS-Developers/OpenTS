---
title: Limit every promotion with VeteranCap
category: balance
release: 0.2.0
targets:
- type: key
  id: VeteranCap
  effect: changed
credit:
- JoyfulShush
---

`VeteranCap` under `[General]` in `rules.ini` now limits every promotion, not only experience from kills. With its default of `1`, armories, veterancy crates, `InitialVeteran`, a reinforcement's `VeteranLevel`, the Drop Pods superweapon, the Make Elite trigger action, and experience set by a map's placed-object record stop at veteran. A Make Elite trigger on an object that is already elite drops it to veteran. Set the value to `2` or higher to let those sources produce elite objects.
