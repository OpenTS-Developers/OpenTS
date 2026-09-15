---
key: GDIFirestormGenerator
summary: The BuildingType whose loss discharges a house's firestorm superweapon and brings down its wall.
see_also: [FirestormWall, "system:laser-fences", "system:superweapons"]
when_omitted:
  kind: value
  value: none
---

A structure of exactly this type being taken off the map or switched off makes its house recount the generators it has left. The recount is skipped while the wall is down. If the house is left with no structure of the type that still qualifies, every [`Type=Firestorm`](/keys/type/) weapon the house holds is [discharged](/systems/superweapons/#the-firestorm-defense) and the wall comes down. For a house under human control that is the same discharge a second cameo click performs, so the drain the wall had not yet spent is turned back into charge.

The test names one type outright rather than asking which structure granted the weapon. [Losing the generator](/systems/laser-fences/#losing-the-generator) lists what makes a structure qualify, and covers what a rules set faces when it grants the defense from somewhere else.
