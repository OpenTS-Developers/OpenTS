---
title: Let armor and firepower crates stack
category: feature
release: 0.2.0
targets:
- type: key
  id: ArmorCrateStacks
  effect: added
- type: key
  id: FirepowerCrateStacks
  effect: added
- type: system
  id: crates
  effect: changed
credit: [ZivDero, dkeeton]
---

`ArmorCrateStacks=yes` and `FirepowerCrateStacks=yes` in `[CrateRules]` of `rules.ini` let an armor or firepower crate upgrade objects that a crate of the same kind already upgraded, multiplying their multiplier again each time. An upgraded collector then also keeps the drawn result instead of receiving money. Both default to `no`, which keeps each upgrade to one crate.

dkeeton is credited for the ts-patches crate patch that first let armor crates stack.
