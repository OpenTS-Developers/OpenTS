---
key: TiberiumChainReaction
summary: Sets off the Tiberium in the cell the animation starts on.
see_also: ["system:tiberium", "TiberiumExplosionDamage", "Debris"]
when_omitted:
  kind: value
  value: "no"
---

The check happens once, as the animation starts: after any delay it was created with, not when it was asked for. It happens only where the cell under it already holds Tiberium. [Damage](/systems/tiberium/#damage) covers what the detonation does to the growth and to whatever is standing in it.

The blast is issued with adjacent growth expressly left alone, so it does not ripple outward the way a Tiberium field set off by a weapon does. One cell goes up, and nothing beyond it is set off in turn.

An animation placed by [Play Anim At](/mapping/actions/taction-play-anim/) is marked inert only after it is created, so the detonation has already run by then. The mark takes effect on any later start: the sound and detonation of a type the animation chains into through [`Next`](/keys/next/) are suppressed.
