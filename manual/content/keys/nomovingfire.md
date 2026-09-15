---
key: NoMovingFire
summary: Stops a vehicle firing while it has somewhere to go, adds a settling delay when it halts, and gates its self-reload.
see_also: ["DeployToFire", "Ammo", "Turret"]
when_omitted:
  kind: value
  value: "no"
---

A vehicle with the flag is refused permission to fire for as long as a destination is assigned to it, whether or not it is actually rolling at that moment. Clearing the destination is what lifts the refusal, so a vehicle stopped mid-route by a blockage is still barred.

Arriving somewhere costs it a little more time. On the step that leaves it with no destination and an empty path, its rearm timer is set to a quarter of what its secondary weapon slot reports as a rearm delay. With no secondary weapon that figure is 1, and the quarter truncates to zero. With an ordinary secondary the burst branch returns a random 3 to 5, so the timer lands on 0 or 1 either way. Only a sonic secondary, or a spark, fire or railgun one with its particle system attached, reports its [`ROF`](/keys/rof/) and produces a delay worth the name.

```ini title="rules.ini"
[MYARTILLERY] ; a UnitType registered in [VehicleTypes]
NoMovingFire=yes
Ammo=2 ; without an ammo limit the reload branch below never runs
```

The flag also gates the only self-reload a vehicle has. A vehicle with an [`Ammo=`](/keys/ammo/) limit and this flag reloads while it is short of full. Its reload timer is held where it stands for as long as the vehicle is moving, and runs down only while it is stopped. Each expiry restores one round, and a vehicle still short of full then waits another 30 seconds for the next. Without the flag that timer is still started by every shot, but nothing ever acts on its expiry. The vehicle then restores no round of its own accord, and a building that reloads units is its only refill.
