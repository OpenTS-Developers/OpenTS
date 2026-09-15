---
key: EMPulseSparkles
summary: The animation attached to an object an EM pulse stuns.
see_also: ["system:emp-pulse"]
when_omitted:
  kind: value
  value: none
---

The key names a single animation. It is attached to every vehicle, aircraft and infantryman [a pulse stuns](/systems/emp-pulse/#what-a-pulse-reaches), and to a stunned structure that is the deployed form of a vehicle. Each starts after a random delay of up to 25 frames. Any other stunned structure receives none. Its length is its own, so it may finish well before the stun does, and on recovery the engine only tells a still-running copy to stop after its current loop.

:::danger[The animation is created without checking that one is named]
Every stun that attaches an animation reads this type. With no type named here the game crashes the first time a pulse stuns one of the objects above.
:::
