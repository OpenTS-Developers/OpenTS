---
key: Floater
summary: Halves the gravity the projectile is flown and aimed under.
see_also: [Gravity, Arcing, Speed]
when_omitted:
  kind: value
  value: "no"
---

Every path that needs a gravity figure for this projectile uses half of [`[AudioVisual] Gravity`](/keys/gravity/) instead of the whole:

- the fall itself;
- the arc an [`Arcing=yes`](/keys/arcing/) weapon solves to aim the shot;
- the reach test that weapon runs before firing;
- the barrel elevation the firer sets while tracking;
- the launch speed derived below.

The launch speed matters most. An unguided weapon does not use the [`Speed`](/keys/speed/#scope-weapontype) written in its section at all. Once the rules are read, the figure is replaced by the speed needed to carry the projectile its full [`Range`](/keys/range/) under the gravity that applies to it. Halving the gravity therefore lowers that speed, and the shot leaves the barrel more slowly and hangs longer in the air over the same distance.

A projectile steered by the homing flight model never falls, so on one of those the setting reaches only the aiming.
