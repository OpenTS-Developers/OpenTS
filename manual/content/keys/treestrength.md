---
key: TreeStrength
summary: Maximum strength a TerrainType takes when its own section sets none.
see_also: [Strength, VeinholeTypeClass]
when_omitted:
  kind: value
  value: "25"
---

A TerrainType is constructed with a maximum strength of `-1` rather than a usable figure. The fallback runs immediately after the type's own section is read: a section that never set [`Strength`](/keys/strength/#scope-aircrafttype), or that set it to `-1` outright, takes this figure instead. `[General]` is read before the object sections, so the value is always in place by then.

The fallback is one-way. Once a type has taken it the stored strength is no longer `-1`, so a later rules layer that omits `Strength` finds the figure already filled in and changes nothing.

:::caution[The fallback needs a section to run in]
It is part of reading a TerrainType's own section, not of creating the type. A type named somewhere in the rules but given no section of its own is never read, so it keeps the `-1` it was constructed with. [`VeinholeTypeClass`](/keys/veinholetypeclass/) explains what that costs when the missing section is a veinhole monster's.
:::
