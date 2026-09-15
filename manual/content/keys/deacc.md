---
key: Deacc
summary: The speed a particle loses each frame.
see_also: ["Velocity", "MaxEC", "BehavesLike"]
when_omitted:
  kind: value
  value: "0"
---

Once a frame this figure is subtracted from the particle's speed, in the same leptons a frame. That speed is a single figure, not one rate per axis of travel. Only two [behaviors](/keys/behaveslike/#scope-particletype) slow down. A `Fire` particle loses the figure every frame until its speed reaches zero, at which point it is flagged for death on the following frame. A `Smoke` particle loses it only while it is still climbing faster than 3 leptons a frame, so a puff settles into a steady rise rather than stopping.

That gives a flame two ways to end. [`Velocity`](/keys/velocity/) divided by this figure is how many frames it can travel, and whichever of that and [`MaxEC`](/keys/maxec/) is shorter is what the flame gets. The stock flame's `28.0` against `0.01` reaches far past its own lifetime. A negative figure adds speed instead: a flame given one never slows to a halt and lives out its full lifetime, accelerating the whole way.
