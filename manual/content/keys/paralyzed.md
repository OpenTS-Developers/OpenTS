---
key: Paralyzed
summary: Keeps a soldier or an armed vehicle on this mission from being given a new one when it runs out of things to do.
see_also: [Zombie, Scatter, NoThreat]
when_omitted:
  kind: value
  value: "no"
---

The setting lives in a mission's own section and is read from the mission the object is currently in.

```ini title="rules.ini"
[Sticky]
Paralyzed=yes
```

A soldier, or a vehicle with a first-slot weapon, that finishes its orders (no target left, nowhere left to go) normally falls back to a guard mission of its own accord. With this set, that fallback is abandoned and the object stays on the mission it is already in. A vehicle with no weapon never reaches the test: the same routine sends it down its own branch, to harvesting, to unloading or to guard, whatever the mission says. Guard and Area Guard reach the same outcome without the flag, so it matters on the missions that are neither.

A vehicle also refuses to scatter while on such a mission, but only partly. The refusal is tested *after* the branch that handles a scatter with no threat coordinate, so a vehicle told to get out of the way of nothing in particular still picks a nearby cell and moves to it. Infantry never test the flag when scattering, and neither do aircraft.

Despite the name, nothing else immobilizes the object. A player order, a team script or an override mission moves it as usual.
