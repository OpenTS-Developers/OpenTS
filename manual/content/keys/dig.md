---
key: Dig
summary: The animation played where a subterranean vehicle breaks the surface.
see_also: [DigSound, TunnelSpeed, Locomotor]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
Dig=MYDIGCLOUD ; an AnimType registered in [Animations]
```

A tunneling vehicle creates the animation at its own position at three points in a trip:

- when it has finished turning to face its destination and starts to dig in
- when the nose-down rotation completes and the descent begins
- when it rises back to within fifty leptons of ground level on the way out

Each of the three also plays [`DigSound`](/keys/digsound/).

A fourth point can play the sound alone. When a vehicle arrives beneath its destination in a cell it cannot enter, or in one outside the local radar, it starts to rise there rather than looking for another spot. The sound plays without an animation, but only if the vehicle is already within fifty leptons of ground level, which a burrowing vehicle is not.

:::danger[An unset animation crashes the game at the first burrow]
The animation is created without first checking that one was named, so with the key unset the game crashes as soon as a subterranean vehicle is given somewhere to go.
:::
