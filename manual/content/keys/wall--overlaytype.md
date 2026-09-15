---
key: Wall
scope: overlaytype
label: Wall overlay
see_also: ["system:walls-and-gates"]
when_omitted:
  kind: value
  value: "no"
---

The flag is what makes a cell overlay behave as a barrier rather than as scenery. It turns on:

- the connection artwork that joins neighboring cells into a run,
- the [damage stages](/systems/walls-and-gates/#taking-damage) that [`Strength`](/keys/strength/#scope-overlaytype) and [`DamageLevels`](/keys/damagelevels/) drive,
- blocked passability: the flag blocks the cell on its own, and a `Wall=no` overlay blocks only when its `Land` value's movement cost is zero,
- the sell cursor,
- eligibility for a [`Wall=yes`](/keys/wall/#scope-warheadtype) warhead.

An overlay left at `no` has none of that: damage passes over it, it no longer blocks as a wall, and no BuildingType can convert into it usefully.

```ini title="rules.ini"
[GAWALL]
Wall=yes
Land=Wall
Strength=100
```

`Wall=yes` is the load-bearing line in that fence: it blocks whatever the `Land` value is, `Clear` included.

It is read separately from the flag of the same name on the BuildingType that lays the overlay. A [`Wall=yes`](/keys/wall/#scope-buildingtype) structure whose [`ToOverlay`](/keys/tooverlay/) names an overlay without this flag still converts into that overlay. The result is an ordinary decorative overlay; any blocking it does comes from its own `Land` value, not from this flag.
