---
key: AnimList
summary: The impact animations of the warhead, chosen by the damage the blast deals.
see_also: [EMEffect, Conventional, SplashList, C4Warhead, IonStormWarhead, DropPodWeapon]
when_omitted:
  kind: value
  value: none
---

Entries divide the damage into 25-point bands in list order: the first covers 1 to 24 points, the second 25 to 49, and the last covers everything above its own band. A blast dealing exactly zero damage plays nothing whatever the list holds, and so does a warhead whose list is empty.

```ini title="rules.ini"
[MyShellWH] ; example WarheadType
AnimList=MYBANG16,MYBANG24,MYBANG34 ; 1-24, 25-49, 50 and above
```

Two other settings take the choice away from that ladder. [`EMEffect=yes`](/keys/emeffect/) draws an entry at random rather than by damage, and [`Conventional=yes`](/keys/conventional/) replaces the list with the shared splash list for a blast that lands on water.

A name the animation list does not declare is registered as a new animation rather than refused, so a misspelled entry produces an impact with no visible explosion instead of an error.

:::danger[Three warheads must have a list of their own]
Most explosions check for a missing animation before building one, but the blasts raised by [`C4Warhead`](/keys/c4warhead/), by [`IonStormWarhead`](/keys/ionstormwarhead/) and by the weapon [`DropPodWeapon`](/keys/droppodweapon/) names do not. Leaving any of those three with an empty list crashes the game the first time one of their blasts occurs. For `C4Warhead` that includes every Tiberium chain reaction that goes off.
:::

:::danger[A healing weapon reads past the front of the list]
Damage below zero is not clamped before the band is worked out. A weapon whose [`Damage`](/keys/damage/#scope-weapontype) is `-25` or lower selects an entry from before the start of the list and hands the resulting pointer straight to the animation system, and the game crashes on impact. Between `-1` and `-24` the first entry is selected as normal.
:::
