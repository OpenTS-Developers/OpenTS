---
key: HunterSeeker
scope: side
label: Side hunter-seeker drone
see_also: [HSBuilding, "system:superweapons"]
when_omitted:
  kind: computed
  note: The first side takes GDIHunterSeeker and the second NodHunterSeeker, as each rules file sets them; any other side names none and launches nothing.
---

```ini title="rules.ini"
[GDI] ; the section carrying the side's own name
HunterSeeker=GHUNTER
```

The UnitType a [hunter-seeker superweapon](/systems/superweapons/#hunter-seeker) creates when a house playing for this side fires it. The drone appears at the cell chosen from [`HSBuilding`](/keys/hsbuilding/), facing east, and then acquires its own target. A side that names no drone spends the charge and launches nothing.
