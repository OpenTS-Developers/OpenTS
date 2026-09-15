---
key: MinZVelocity
summary: The smallest upward speed a spark is thrown at.
see_also: ["ZVelocityRange", "XVelocity", "YVelocity", "BehavesLike"]
when_omitted:
  kind: value
  value: "0"
---

This figure is the floor of the vertical part of a [`Spark`](/keys/behaveslike/#scope-particletype) particle's initial speed, in leptons a frame. Each spark is thrown upward at this figure plus a random amount below [`ZVelocityRange`](/keys/zvelocityrange/). The stock `40` with a range of `15` therefore throws every spark between 40 and 54 leptons a frame, before the holding system folds in its own direction. Only sparks read it. The vertical spread of a burst is entirely this floor and that range. The figure takes no part in the one random direction a burst shares: that direction draws each axis from `XVelocity`, `YVelocity` and `ZVelocityRange` alone, as a signed draw taken modulo the key, so any component can come out negative.

The rest of a spark's flight is gravity. [`Gravity`](/keys/gravity/) is taken off the vertical speed once a frame, which is what arcs the shower over. The spark dies where it meets ground, a bridge deck or a wall.

:::note[Sparks fall one gravity step faster than their stored speed]
The pull is applied twice each frame to the distance a spark covers but only once to the speed it takes into the next frame. Every spark therefore travels as though it were already a frame further into its fall, which lowers the apex of the arc below what this figure and the range would otherwise produce.
:::
