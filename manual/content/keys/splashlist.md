---
key: SplashList
summary: The animations played where something comes down in water.
see_also: ["Wake", "Conventional", "IsMeteor"]
when_omitted:
  kind: value
  value: ""
---

Which entry is used depends on what made the splash, and only one of the paths looks at the list as a whole.

An explosion over open water picks by its damage: the first entry for the first thirty-five points, the next for the thirty-five after that, and so on up the list, with the last entry standing for everything past what the list covers. The warhead must be [`Conventional=yes`](/keys/conventional/) and the point must not be over a bridge deck. The splash replaces the warhead's own explosion rather than joining it.

Every other path takes one fixed entry and ignores the damage. The first entry goes with an ordinary bouncing animation, a piece of voxel debris, a burning infantryman and infantry destroyed over water. The last entry goes with a destroyed vehicle, an ion cannon blast that lands on water, and an `IsMeteor=yes` impact of either kind — the [animation](/keys/ismeteor/#scope-animtype) and the [voxel animation](/keys/ismeteor/#scope-voxelanimtype) both take it. About half of the eight lay a [`Wake`](/keys/wake/) down alongside the splash; the two meteor kinds, an ion cannon blast and a burning infantryman do not.

```ini title="rules.ini"
[CombatDamage]
SplashList=MYSPLASH1,MYSPLASH2,MYSPLASH3 ; AnimTypes registered in [Animations]
```

:::danger[An empty list crashes the game at the first splash]
Only the explosion path checks that the list holds anything. Every other path reads an entry from it outright, so a rules file that never sets the key at all leaves the list empty and the game crashes the first time anything falls in the water. Leaving the key out of a later rules layer is harmless; the list keeps whatever an earlier one set.
:::
