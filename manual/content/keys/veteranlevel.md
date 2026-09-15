---
key: VeteranLevel
summary: The rank every member created for this team starts at.
see_also: ["system:veterancy"]
when_omitted:
  kind: value
  value: "1"
---

The rank is assigned to each TaskForce member as it is created for a reinforcement group, before the members are sorted into transports and passengers. Transports and their cargo alike take it.

| Value | Rank the member starts at |
| --- | --- |
| `0` | Below rookie, at `-0.25` experience |
| `1` | Rookie, as created |
| `2` | Veteran |
| `3` | Elite |

Any other value leaves the member at the rank it was created with, exactly as `1` does. The ceiling in [`VeteranCap`](/keys/veterancap/) is not read, so a team can be delivered elite even where combat promotion stops at veteran.

`0` is the only team or rules setting that produces negative experience; beyond the settings, a map's placed-object records can write the experience figure directly. In combat the state costs the member nothing: a below-rookie object holds no abilities, just as a rookie does not. It shows only in the [insignia it draws](/systems/veterancy/#rank-display) and in how a veterancy crate and an armory treat it.
