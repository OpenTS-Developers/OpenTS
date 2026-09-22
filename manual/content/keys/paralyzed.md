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

When a soldier, or a vehicle with a first-slot weapon, runs out of orders (no target left and nowhere left to go), it normally switches to a guard mission. If its current mission sets `Paralyzed=yes`, it stays on that mission instead. Vehicles with `Harvester=yes` or `Weeder=yes`, and vehicles without a weapon, ignore the setting: when idle, they go back to harvesting, unload or take guard, whatever their mission. An object on Guard or Area Guard stays on that mission anyway, so the setting matters only on other missions.

A vehicle also refuses to scatter while on such a mission, but only partly. The refusal is tested *after* the branch that handles a scatter with no threat coordinate, so a vehicle told to get out of the way of nothing in particular still picks a nearby cell and moves to it. Infantry never test the flag when scattering, and neither do aircraft.

Despite the name, nothing else immobilizes the object. A player order, a team script or an override mission moves it as usual.
