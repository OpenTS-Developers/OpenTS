---
key: IsVeinholeMonster
summary: Lets an explosion collect the veinhole monster standing under the overlay.
see_also: ["system:veins", "IsVeins"]
when_omitted:
  kind: value
  value: "no"
---

An explosion builds its victim list from the cell it is centered on and the eight around it. Finding this overlay on any of those nine cells is what makes it look for a [veinhole monster](/systems/veins/#destruction) as well. The lookup itself names the blast's own cell, so the monster is collected only when the explosion is centered on the cell the monster occupies. Nothing else in the engine reads the flag.
