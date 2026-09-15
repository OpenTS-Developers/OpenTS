---
key: Arcing
summary: Aims the weapon by solving a ballistic arc to the target instead of pointing straight at it.
see_also: [Floater, Inaccurate, Bouncy, Lobber, Gravity]
when_omitted:
  kind: value
  value: "no"
---

The setting changes how the shot is aimed and how the weapon's reach is judged. It does not choose the flight model; that follows from [`ROT`](/keys/rot/#scope-bullettype), and a projectile with a rate of turn is steered by the homing flight model whatever this setting says.

**Aiming.** The launch pitch is solved from the horizontal distance to the predicted target position, the height difference, the launch speed and gravity. The high solution is taken when the firing weapon is [`Lobber=yes`](/keys/lobber/), and also whenever the target is above the firer and closer horizontally than it is high; otherwise the flat solution is taken. Gravity is [`[AudioVisual] Gravity`](/keys/gravity/), halved for a [`Floater=yes`](/keys/floater/) projectile. When no arc at that speed reaches the target the shot is abandoned: no projectile leaves the firer, no ammunition is spent and no reload delay is started, so the firer tries again immediately.

**Reach.** An arcing weapon's range test is not a distance comparison. The target must lie where an arc at the weapon's launch speed can reach it, and a target standing in a bridge cell three or more terrain levels above the firer is refused outright.

**Clearance.** The arc clears what lies under it only while the shot passes high enough. A [`High=yes`](/keys/high/#scope-overlaytype) overlay detonates a shot that crosses its cell below height 100. A shot with no rate of turn also strikes any wall or building standing in a cell it crosses less than 150 leptons above the ground. Higher than that, the lob flies on over them.

**Detonation.** A projectile that goes off before it arrives is normally moved to the point its fuse was aimed at; an arcing one is left where it went off.

Pairing the setting with [`Inaccurate=yes`](/keys/inaccurate/) is what makes a shot scatter: the scatter is applied to the aim point before the arc is solved, and neither setting scatters anything on its own.
