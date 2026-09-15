---
key: DeployToFire
summary: Restricts a vehicle's fire to ground it could deploy on, and stops a player's copy picking targets for itself.
see_also: ["DeploysInto", "NoMovingFire", "Buildable"]
when_omitted:
  kind: value
  value: "no"
---

The flag ties firing to ground the vehicle could deploy on. A cell qualifies under **All of:**

- it has no ramp;
- it holds no building;
- its land type has the [`Buildable`](/keys/buildable/) entry set.

From any other cell, the vehicle refuses to fire.

Standing somewhere it cannot deploy, the vehicle holds the shot back whether the weapon was clear to fire or merely needed to turn. It moves toward the target until it stands on ground it can fire from. The game adds no cue of its own: no special cursor marks the order, and the vehicle driving toward the target instead of firing is the only feedback.

```ini title="rules.ini"
[MYSIEGETANK] ; a UnitType registered in [VehicleTypes]
DeployToFire=yes
DeploysInto=MYSIEGEGUN ; a BuildingType registered in [BuildingTypes]
```

A vehicle looking for somewhere to shoot from restricts its search to deployable ground whenever the cell it currently stands on is not deployable. The range it will search is narrowed to two cells beyond its distance from the target, so it settles close instead of wandering to the edge of its reach.

Under a human player the flag also switches off target acquisition entirely: such a vehicle never chooses a target of its own and only ever shoots what it is ordered to. A computer-controlled one keeps its ordinary target search. That restriction belongs to the flag alone. A vehicle that deploys before firing only because its [`DeploysInto`](/keys/deploysinto/) structure is a [`TickTank=yes`](/keys/ticktank/) one still picks its own targets.
