---
key: Zombie
summary: Holds an object on this mission when it runs out of things to do, and after damage if the mission is also marked no threat.
see_also: [NoThreat, Paralyzed, Retaliate]
when_omitted:
  kind: value
  value: "no"
---

The setting lives in a mission's own section and is read from the mission the object is currently in. It does two things, and both are about refusing to leave the mission.

```ini title="rules.ini"
[Sleep]
Zombie=yes
```

The first is the one that pairs with [`NoThreat`](/keys/nothreat/). A vehicle, soldier or aircraft that belongs to no team is snapped out of a mission marked no threat the moment damage from an identified attacker lands without destroying it. The flag that hides it from target scans is treated as a state it should not stay in once it has been found. This setting cancels that, so an object that is meant to sit still and be ignored goes on sitting still after it is shot. The no-threat flag is tested first at the one place this half is read, so it does nothing on a mission that is not also marked no threat. A team member never reaches the test at all: damage to one is reported to its team instead.

The second is shared with [`Paralyzed`](/keys/paralyzed/): a soldier, or a vehicle with a first-slot weapon, that has finished its orders is not given a guard mission in place of this one, and stays where it is.

Nothing here stops the object from firing back. Whether it acquires the attacker is [`Retaliate`](/keys/retaliate/), which is tested separately.
