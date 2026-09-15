---
key: Explodes
scope: overlaytype
label: Explosive overlay
see_also: [BarrelExplode, BarrelDebris, BarrelParticle, AmmoCrateDamage, C4Warhead, ChainReaction]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[MYBARREL] ; an OverlayType registered in [OverlayTypes]
Explodes=yes
```

An explosion whose center lands in the cell sets the overlay off. There is no strength test, no armor test and no chance roll. The segment is removed, the cell recalculates its passability and zones, and anything aiming at it drops the target. The detonation follows at once. It is the [`[AudioVisual] BarrelExplode`](/keys/barrelexplode/) animation, area damage of [`[CombatDamage] AmmoCrateDamage`](/keys/ammocratedamage/) with [`C4Warhead`](/keys/c4warhead/) and no owner, one entry from [`BarrelDebris`](/keys/barreldebris/) on a 15% roll, and a [`BarrelParticle`](/keys/barrelparticle/) system on a 25% roll.

That debris roll walks the list in order and stops at the first piece it creates. At most one piece is ever thrown, and later entries are only reached when every earlier one failed its roll.

Each of the four neighboring cells that also holds an explosive overlay receives a `FIRE3` animation four to six frames later. The neighbor is not detonated directly; whether the fire goes on to set it off depends on that animation's own [`Damage`](/keys/damage/#scope-animtype). The same fire is laid on explosive neighbors when an [`Explodes=yes`](/keys/explodes/#scope-aircrafttype) structure is destroyed.

This is a separate mechanism from [`ChainReaction=yes`](/keys/chainreaction/), which is what spreads a Tiberium field's own destruction. An overlay with both flags runs both on one hit: the chain reaction is judged first, and this detonation follows it.

:::danger[The `FIRE3` animation is required]
The neighbor fire is looked up by name every time, and a name that is not registered in `[Animations]` resolves to an index of `-1` that is used to index the animation list anyway. Removing or renaming `FIRE3` therefore reads outside that list and builds an animation from whatever it finds.
:::
