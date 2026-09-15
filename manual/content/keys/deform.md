---
key: Deform
summary: The chance, in percentage points per point of damage, that a blast lowers the ground under it.
see_also: [DeformThreshhold]
when_omitted:
  kind: value
  value: "0"
---

The chance that a blast craters is the damage it deals multiplied by this figure, read as a percentage and truncated to a whole number of percent. A product below one never fires at all. A value written with a trailing percent sign is divided by 100 first, which is what makes `Deform=15%` fifteen hundredths of a percentage point per damage point.

```ini title="rules.ini"
[MyShellWH] ; example WarheadType
Deform=15%
DeformThreshhold=120
```

At 200 damage that warhead craters three times in ten; at 120 damage or less it never craters, because the roll is made only above [`DeformThreshhold`](/keys/deformthreshhold/). The figure tested is the raw damage of the blast, not the amount any one object ends up taking.

A crater drops its cell one height level. The change arrives in two parts: one corner group at the moment of the blast, and the rest five frames later. The whole thing is refused when any of the eight cells around the target lies outside the playable area. An ordinary explosion standing above a bridge deck never craters the ground beneath it.

Three wide-area explosions make the same roll from the damage at their center before that damage is spread outward: a nuclear detonation, a loaded harvester going up, and the collateral blast of an [`Explodes=yes`](/keys/explodes/#scope-aircrafttype) object. They make it whether or not they stand over a bridge.
