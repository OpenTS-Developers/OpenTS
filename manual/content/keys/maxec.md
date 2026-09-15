---
key: MaxEC
summary: The base lifetime of a particle, in frames.
see_also: ["MaxDC", "DeleteOnStateLimit", "NextParticle", "BehavesLike"]
when_omitted:
  kind: value
  value: "1"
---

A particle is created with this many frames to live plus a random extra, and is removed when the count reaches zero. For a [`Railgun`](/keys/behaveslike/#scope-particletype) particle the extra is a fixed 0 to 9 frames. For every other behavior it is a random amount smaller than this value itself. A cloud created in one burst therefore thins out over a spread of frames roughly as wide as the lifetime rather than vanishing all at once. Frames run fifteen to the second, so `900` in `MaxEC` gives a particle a minute to live.

Expiring is not the only trigger for a [`NextParticle`](/keys/nextparticle/) successor: any death does it, so a particle killed early by [`DeleteOnStateLimit`](/keys/deleteonstatelimit/) or by drifting up into a bridge deck spawns its successor just the same.

:::danger[A lifetime of zero stops the game]
The random extra is drawn by dividing by this value, so `MaxEC=0` divides by zero and the game stops the moment the first particle of the type is created. Only `Railgun` particles escape it, because their extra is drawn against a fixed figure instead.
:::

:::caution[The countdown holds sixteen unsigned bits]
A negative lifetime, or a lifetime and extra that together exceed 65,535, wraps rather than clamping. The usual result of a negative figure is a particle that lives out almost the full 65,535 frames (over an hour of game time) instead of dying at once. A system that waits for its last particle to die waits that long with it.
:::
