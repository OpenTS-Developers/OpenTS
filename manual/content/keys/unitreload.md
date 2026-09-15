---
key: UnitReload
summary: Makes the building rearm the object docked with it, one ammunition point at a time.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "no"
---

A docked client gains one point of [`Ammo`](/keys/ammo/) per [`ReloadRate`](/keys/reloadrate/) interval, 45 frames at the default, until its magazine is full. Nothing is repaired and nothing is charged. A helipad rearms the aircraft that land on it only if it sets this flag as well.

The branch is separate from [`UnitRepair`](/keys/unitrepair/) and a building runs only [the first service branch its flags match](/systems/repair/#unitreload-is-a-different-service), so a type with both repairs and never rearms. A second aircraft asking to dock is not simply refused, though: a parked aircraft whose magazine is already full is sent to a nearby cell to make room for it.
