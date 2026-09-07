---
key: Explosion
summary: The animations a destroyed object leaves behind, one drawn at random from the list.
see_also: [ScrapExplosion, ScrapMetal, Explodes, DebrisTypes, MaxDebris, FirestormWarhead, Ammo]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
Explosion=TWLT070,FRAG1,FRAG3 ; AnimTypes registered in [Animations]
```

Each object kind spends the list differently:

- A **vehicle** creates one entry, picked at random, at its own position — unless its art declares [`DeathFrames`](/keys/deathframes/), which defers the animation to the end of its death throes; unless it was killed by the firestorm warhead, which substitutes seven to nine firestorm particle systems; or unless it fell and died over water, which leaves a wake and the last splash entry instead. A vehicle that [`Explodes=yes`](/keys/explodes/#scope-aircrafttype) — or carries the veteran explodes ability — takes the **last** entry instead whenever it still has ammunition, meaning an unlimited [`Ammo`](/keys/ammo/) pool or a count above zero, so the biggest wreck belongs at the end of the list.
- A **structure** creates one entry, picked afresh, on *every* cell of its footprint, scattered up to a quarter cell from the cell center, each starting between zero and three frames later. A large structure therefore consumes the list once per cell, not once per building.
- An **aircraft** creates one entry, picked at random, at its center — but nothing at all when the killing warhead is [`[CombatDamage] FirestormWarhead`](/keys/firestormwarhead/), which substitutes seven to nine firestorm particle systems.
- An **InfantryType** stores the list and never reads it back.

A match played with [`ScrapMetal`](/keys/scrapmetal/) on spends the type's [`ScrapExplosion`](/keys/scrapexplosion/) list instead of this one, in every way described above; a type that names no scrap animations keeps these.

An empty list simply leaves out the animation; the debris, the death voice and any [`Explodes=yes`](/keys/explodes/#scope-aircrafttype) blast are unaffected either way.

Writing `Explosion=none` empties the list, because the placeholder resolves to no type and nothing is left to add. Writing the key with nothing after the `=` is a different thing: the read finds no value and keeps whatever an earlier rules file set. A name the game does not already know is registered as a new animation type rather than rejected, so a misspelling becomes an animation of its own.
