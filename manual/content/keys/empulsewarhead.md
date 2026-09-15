---
key: EMPulseWarhead
summary: Parsed WarheadType that the engine never uses.
no_effect: true
see_also: [EMPulseProjectile, EMPulseCannon, "system:emp-pulse"]
when_omitted:
  kind: value
  value: none
---

The name promises the warhead an EM pulse cannon detonates. [Firing the cannon](/systems/emp-pulse/#em-pulse-cannon-superweapon) instead fires the building's own primary weapon, so the pulse comes from that weapon's warhead. Nothing beyond the save file and the cleanup that clears the pointer when a warhead is deleted touches the value stored here.
