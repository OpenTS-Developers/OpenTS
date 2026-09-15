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
[GDI] ; the section matching the side's own name
HunterSeeker=GHUNTER
```

The UnitType a [hunter-seeker superweapon](/systems/superweapons/#hunter-seeker) creates when a house playing for this side fires it. The drone appears near the house's building listed in [`HSBuilding`](/keys/hsbuilding/), facing east, and then acquires its own target. A side that names no drone spends the charge and launches nothing. The launch demands nothing of the type, but only a type that sets [`HunterSeeker=yes`](/keys/hunterseeker/) hunts a victim down and detonates as a drone; any other type arrives behaving as the ordinary unit it is.
