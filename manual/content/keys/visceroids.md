---
key: Visceroids
summary: Parsed flag that spawns nothing.
no_effect: true
see_also: [TiberiumDeathToVisceroid, SmallVisceroid]
when_omitted:
  kind: value
  value: "no"
  note: A scenario that omits the key uses this built-in default.
---

Nothing in the game reads this flag, so it does not decide whether infantry killed by Tiberium leave a small visceroid. [`TiberiumDeathToVisceroid`](/keys/tiberiumdeathtovisceroid/) in the map's `[Basic]` section decides that.
