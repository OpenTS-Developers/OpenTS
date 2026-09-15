---
key: IsSonic
summary: Sends a visible wave rolling from the muzzle to the target, damaging everything it crosses on the way.
see_also: ["AmbientDamage", "ROF", "Range", "Warhead"]
when_omitted:
  kind: value
  value: "no"
---

The wave takes the shot's damage over entirely. A sonic weapon's projectile is created with its firepower forced to nothing, so [`Damage=`](/keys/damage/#scope-weapontype) is never delivered and everything the weapon does is done by [`AmbientDamage`](/keys/ambientdamage/) as the wave passes.

```ini title="rules.ini"
[MySonicGun] ; example WeaponType
IsSonic=yes
Projectile=MyBolt ; a BulletType, registered by a weapon naming it as its Projectile
AmbientDamage=3
Damage=1  ; never delivered
Range=6
ROF=120
```

The wave grows out from the muzzle by a twentieth of the distance each frame, reaching the target after twenty frames. It then holds at full length for another sixty-one before it begins to fade from its trailing edge. In all it lasts about a hundred frames, some seven seconds. On every one of those frames it damages every object standing in a cell it currently covers, so anything that stays under it takes the ambient figure many times over rather than once. Even a small figure adds up to a great deal. It also wears down wall overlay it crosses and sets off chain reactive overlay.

Neither of the object's weapons may fire while a wave is alive, and the reload delay is exactly [`ROF`](/keys/rof/) with the house's rate of fire bias, the burst gaps and the random padding all skipped. A structure with more than one round left waits a single frame instead, so only the effect's own lifetime paces it.

:::caution[The wave is fired and fed from the first weapon slot]
The damage figure and the warhead the wave applies are read from the weapon in the object's first slot, not from the slot that fired, and the wave is re-anchored each frame to that slot's muzzle. A sonic weapon in the second slot therefore rolls out a wave with the first weapon's ambient damage and warhead.
:::

:::caution[A wave fired beyond about 8.5 cells dies at once]
The wave keeps growing only while the firer is still aiming at the same target and the two are no more than 2172 leptons apart, a little under 8.5 cells. Past that it stops and fades from its trailing edge. A wave that was already past that distance when it was created is removed on the same frame and damages nothing. A sonic weapon whose [`Range=`](/keys/range/) reaches further than that simply misses at its longest shots.
:::
