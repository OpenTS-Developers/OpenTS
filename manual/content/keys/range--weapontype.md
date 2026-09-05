---
key: Range
scope: weapontype
label: Firing distance
see_also: ["MinimumRange", "ProjectileRange", "Speed", "GuardRange", "system:target-selection"]
when_omitted:
  kind: value
  value: "0"
---

The value is written in cells and a fraction is accepted; it is held as 256 units to the cell. Before any comparison the engine takes 85 of those units off the figure — a third of a cell — so the reach is always that much shorter than the number written, and a target sitting exactly at the written distance is out of range.

```ini title="rules.ini"
[MyCannon] ; example WeaponType
Range=10.5
MinimumRange=2
```

The distance compared against it is the full three-dimensional separation between the firing coordinate and the target's center. An AircraftType is the exception and measures the horizontal separation only, ignoring how far below it the target lies. Against a structure the reach is stretched back out by a quarter of a cell for every cell of the structure's width and height added together, so a wide building can be hit from further away than a vehicle.

Three further refusals ride on the same test. The first two apply only to a projectile that is not [`Arcing=yes`](/keys/arcing/): a weapon whose projectile is not [`AA=yes`](/keys/aa/) refuses a target raised above the firer by at least the horizontal distance between them, and a firer standing beneath a bridge cannot reach a target standing on top of one. The third holds whichever projectile is fired: a shot aimed as steeply downward — the firer higher than the target by at least that distance — is refused unless the firer is itself off the ground.

:::caution[An arcing projectile does not use the figure as a range at all]
When the weapon's [`Projectile=`](/keys/projectile/) sets [`Arcing=yes`](/keys/arcing/), the distance comparison is skipped entirely and whether the target can be reached is decided by solving the shot's ballistic arc from its launch speed. That launch speed is itself worked out from this figure for any unguided projectile, so `Range=` still governs the reach — indirectly, through the speed rather than through a limit. [`Speed=`](/keys/speed/#scope-weapontype) covers that substitution.
:::

Outside the range test the figure is read in several places: it is the scan radius an object falls back on when its [`GuardRange`](/keys/guardrange/) is zero and the distance the threat score is measured against, both of which [target selection](/systems/target-selection/#scan-radius) covers; it is capped at four cells when a computer house rates a base defense against armor or infantry; a superweapon cannon takes it, in whole cells, as the reach of its strike; and a weapon named as a projectile's [`AirburstWeapon=`](/keys/airburstweapon/) hands it to each bomblet as fuel rather than as a firing distance.

A figure of exactly `-1` is read as though the key were absent, leaving whatever an earlier rules file set. A figure of `0` leaves the reach at less than nothing, so an ordinary weapon set that way can never fire at all.
