---
key: Explodes
scope: aircrafttype
label: Violent death
see_also: [CollateralDamageCoefficient, Explosion, ExpSpread, Primary, MaxDebris]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[MYAMMOTRUCK] ; a UnitType registered in [VehicleTypes]
Explodes=yes
```

A destroyed object of this type sets off a blast that damages everything around it. An object whose rank grants the `EXPLODES` ability through [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/) sets off the same blast without this key.

The blast uses the warhead of the object's first weapon slot. That is [`Primary`](/keys/primary/), or [`Elite`](/keys/elite/) once the object is elite, or an upgrade's weapon on a structure that has one. An object with no weapon in that slot sets off no blast: it deals no damage and plays no animation or flash.

The blast's strength is the object's collateral damage: its full [`Strength`](/keys/strength/#scope-aircrafttype) multiplied by [`CollateralDamageCoefficient`](/keys/collateraldamagecoefficient/). Damage the object had already taken does not reduce it, and a structure adds the Tiberium it stores. The blast plays a combat explosion animation chosen by that strength, the warhead and the ground under the object, and adds a lighting flash when the warhead is [`Bright=yes`](/keys/bright/#scope-warheadtype). The damage is credited to whatever destroyed the object.

The radius in cells is the collateral damage divided by 100 and rounded down, then divided by [`[CombatDamage] ExpSpread`](/keys/expspread/). It is held between one lepton and three cells. The damage dealt is the collateral damage multiplied by the radius in whole cells, and never less than the collateral damage itself. Collateral damage below 100 therefore gives the smallest blast. `ExpSpread` covers the cells the blast reaches and how its damage is shared among them.

The key changes three more parts of the death:

- **Spilled Tiberium.** A vehicle, aircraft or infantry soldier carrying Tiberium, such as a harvester, spills its load around it, even when it has no weapon and sets off no blast. A scenario with [`HarvesterImmune=yes`](/keys/harvesterimmune/) spills nothing. [Spilled harvester loads](/systems/destruction-and-debris/#spilled-harvester-loads) covers the amount and where it lands. The `EXPLODES` ability has the same effect.
- **Vehicle death animation.** A vehicle that still has ammunition plays the last entry of its [`Explosion`](/keys/explosion/) list instead of a random one. The `EXPLODES` ability has the same effect.
- **Structures.** A structure places a `FIRE3` fire animation on any [explosive overlay](/keys/explodes/#scope-overlaytype) in the four cells that share an edge with the top corner cell of its footprint. It also stays on the map until its next update, which [releases its survivors a second time](/systems/destruction-and-debris/#when-the-structure-leaves-the-map) unless it is being sold. The `EXPLODES` ability does neither.

A vehicle or infantry soldier that falls into water leaves only a splash. It falls when a bridge collapses under it, or when it is a hover vehicle that sinks too low over water. It sets off no blast and spills no Tiberium. An aircraft shot down in flight is not falling in this sense, so its blast goes off where it was destroyed, even over water.
