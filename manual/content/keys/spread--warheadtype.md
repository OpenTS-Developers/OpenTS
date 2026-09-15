---
key: Spread
scope: warheadtype
label: Damage falloff
see_also: ["system:emp-pulse"]
when_omitted:
  kind: value
  value: "1"
---

Damage falls off with the distance from the impact point divided by this factor, so a larger value spreads the same damage further rather than adding damage.

The figure is not how far the blast reaches. How much ground an ordinary blast gathers is fixed at a cell and a half whatever this is set to, and raising it only makes the damage thin out more slowly across that same ground. [Warheads and damage](/systems/warheads/) covers the reach and the steps the falloff runs in. The figure does set a radius in one other place: an [`EMEffect=yes`](/keys/emeffect/) warhead takes the same figure as [the pulse radius in cells](/systems/emp-pulse/#firing-a-pulse). A pulse's reach and its falloff cannot be set apart.

```ini title="rules.ini"
[MyPulseWH] ; a WarheadType with EMEffect=yes
Spread=4 ; the pulse reaches four cells out from its center
```

