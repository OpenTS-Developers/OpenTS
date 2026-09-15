---
key: TreeFlammability
summary: Chance a burning terrain object sets fire to each unburnt terrain object beside it.
when_omitted:
  kind: value
  value: ".1"
---

The figure is a fraction from 0 to 1, so the stock `.05` gives each qualifying neighbor a one-in-twenty chance. At `1` or above every qualifying neighbor catches on every attempt.

A burning terrain object tries to spread the fire on roughly one frame in a hundred, and nothing configures that interval. On a frame it does, each of the eight neighboring cells holding a terrain object that is not already alight is drawn against separately. A tree hemmed in on every side gets eight independent chances at once.

Winning the draw is not enough to catch. The neighbor has to satisfy **All of:**

- it is not already crumbling, the animation a destroyed terrain object plays out before it disappears;
- its type sets [`Armor=wood`](/keys/armor/#scope-aircrafttype), which is what a TerrainType has unless its section sets something else;
- its type does not declare [`SpawnsTiberium=yes`](/keys/spawnstiberium/), so a blossom tree never burns.

The only other way a terrain object catches is a [`Sparky=yes`](/keys/sparky/) warhead damaging it, and that path clears the same three conditions.
