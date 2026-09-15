---
key: Acceleration
scope: bullettype
label: Homing projectile acceleration
see_also: [ROT, Speed]
when_omitted:
  kind: value
  value: "3"
---

Only a projectile steered by the homing flight model reads the figure: one whose [`ROT`](/keys/rot/#scope-bullettype) is above zero. A projectile with no rate of turn follows the arc it was launched along and never changes speed under its own power.

The figure is leptons of speed gained per game frame, added once every frame until the projectile reaches the speed the firing weapon allows. There are 256 leptons to a cell and 15 game frames to the second. The figure only decides how quickly the projectile reaches the weapon's speed, never how fast it ends up flying: the gain is clamped to that ceiling the moment it is reached.

A projectile leaves the launcher at one lepton per frame. While it is still in that launch phase the setting is put aside entirely: the speed rises by one lepton on even game frames and not at all on odd ones. The projectile therefore eases away from its firer at half a lepton per frame however large the figure. The launch phase ends as soon as the projectile is within half a lepton of the weapon's speed, and a weapon whose [`Speed`](/keys/speed/#scope-weapontype) is `16` or more has no launch phase at all.

:::caution[A fast weapon with no acceleration leaves its projectile crawling]
The launch ramp is the only thing that moves a projectile set to `Acceleration=0`, and a weapon whose `Speed` is `16` or more skips that ramp. Such a projectile is left at its launch speed of one lepton per game frame, about a seventeenth of a cell each second, for the whole flight.
:::
