---
key: IgnoresFirestorm
scope: aircrafttype
label: Object types
see_also: ["system:laser-fences"]
when_omitted:
  kind: value
  value: "no"
---

Four separate firestorm tests read this flag:

- the sweep of a raised section's own cell
- the sweep of the cells around it for anything walking toward it
- the crossing test a flying object runs as it passes over a section
- the check a projectile makes on entering a section's cell

An exempt type is skipped by all four. [What a raised section destroys](/systems/laser-fences/#what-a-raised-section-destroys) covers the payload it is being spared.

Exemption is per type, not per object, and it does not extend to anything else the wall does. A projectile type whose flight is invisible is tested on a different path that does not read this flag, so it is consumed regardless. An exempt vehicle still cannot path through a raised section's cell, because the cell is impassable to movement whatever stands on it.

:::caution[A jumpjet is destroyed anyway]
Which of the two crossing tests a type meets follows from its [`Locomotor=`](/keys/locomotor/) setting: the Flyer locomotion class runs the test that reads this flag, and the Jumpjet locomotion class runs one that does not. A type on the Jumpjet class with `IgnoresFirestorm=yes` is still destroyed when it moves onto a raised section's cell.
:::
