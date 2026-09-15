---
key: Bouncer
summary: Throws the animation into the air on a random arc instead of playing it where it was created.
see_also: ["IsMeteor", "MaxXYVel", "MinZVel", "Elasticity", "ExpireAnim", "Spawns"]
when_omitted:
  kind: value
  value: "no"
---

The animation is launched from 10 leptons above the coordinate it was asked for. Its two horizontal speeds are drawn from the [`MaxXYVel`](/keys/maxxyvel/#scope-animtype) range and its upward speed from [`MinZVel`](/keys/minzvel/#scope-animtype). Gravity then takes 1.4 leptons per frame off the vertical speed on every frame of the flight. [`IsMeteor=yes`](/keys/ismeteor/#scope-animtype) puts an animation into the same physics without this flag and takes over the launch when both are set.

The flight ends on the first contact, whether that is the ground, a bridge deck, or a building or wall standing in the way. Everything the type sets for its landing happens on that single frame, and the animation removes itself afterward: [`ExpireAnim`](/keys/expireanim/#scope-animtype) with the blast inside it, [`ExpireSound`](/keys/expiresound/#scope-animtype), [`Spawns`](/keys/spawns/#scope-animtype) and [`IsTiberium`](/keys/istiberium/#scope-animtype). A landing low over water is different: a wake and a splash replace the expiry animation, the expiry sound does not play, and [`Spawns`](/keys/spawns/#scope-animtype) and [`IsTiberium`](/keys/istiberium/#scope-animtype) are skipped. Whether the same contact also counts as a strike, playing [`BounceAnim`](/keys/bounceanim/#scope-animtype) and [`BounceSound`](/keys/bouncesound/#scope-animtype) and dealing the [`DamageRadius`](/keys/damageradius/#scope-animtype) damage, is what [`Elasticity`](/keys/elasticity/#scope-animtype) decides.

A thrown animation is exempt from two things an ordinary one does. It deals none of the per frame [`Damage`](/keys/damage/#scope-animtype) an animation applies to whatever it is sitting on. It reaches the largest frame test that lays a [`Crater`](/keys/crater/#scope-animtype) or a [`Scorch`](/keys/scorch/) only where that largest frame is its first.

```ini title="art.ini"
[MYDEBRIS]         ; a chunk thrown off a destroyed vehicle
Bouncer=yes
MaxXYVel=30.0      ; up to 30 leptons a frame sideways
MinZVel=20.0       ; at least 20 leptons a frame upward
LoopCount=-1       ; loop until the chunk lands
ExpireAnim=TWLT026 ; the stock small impact flash
```

:::caution[A thrown animation still runs out of frames]
Bouncing does not suspend the animation's own playback. One that reaches the end of its frames is removed wherever it has got to, in mid-air if the flight is not over. A type meant to be thrown any distance needs [`LoopCount=-1`](/keys/loopcount/) to keep looping until it lands. Every bouncing animation in the shipped `art.ini` declares it.
:::
