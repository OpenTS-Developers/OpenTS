---
key: HSBuilding
summary: The BuildingTypes a hunter-seeker drone may walk out of.
see_also: [GDIHunterSeeker, NodHunterSeeker, "system:superweapons"]
when_omitted:
  kind: value
  value: ""
---

Firing a [`Type=HunterSeeker`](/keys/type/) weapon scans every structure the house owns against this list and keeps the *last* match, not the first. The drone therefore leaves whichever qualifying structure was created most recently. The drone then appears at the nearest cell to that structure that infantry could stand in, and only when that cell lies inside the playable area.

```ini title="rules.ini"
[SpecialWeapons]
HSBuilding=GAPLUG,NATMPL   ; GDI Upgrade Center and Nod Temple of Nod
```

Nothing is created, and the discharge still spends the charge, when **any of:**

- the list is empty;
- the house owns none of the named types;
- the side's [`HunterSeeker=`](/keys/hunterseeker/#scope-side) names no drone.
