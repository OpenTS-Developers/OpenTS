---
key: Normalized
scope: particletype
label: Particle state pacing
see_also: ["StateAIAdvance", "FinalDamageState"]
when_omitted:
  kind: value
  value: "no"
---

A particle is always created aimed from one point at another: an origin it starts at, and a target its motion vector points at. Marking the type `Normalized=yes` discards the particle's own [`StateAIAdvance`](/keys/stateaiadvance/) and recomputes it as the particle is created. The engine works out how long the particle needs to cross that origin-to-target distance at its current speed, divides that by one more than [`FinalDamageState`](/keys/finaldamagestate/) and adds one. The result, truncated to a whole number of frames between state advances, is the interval the particle runs on, so it finishes its damage-state sequence just as it arrives, however far it has to travel. Only a `Fire` particle runs on that recomputed interval; `StateAIAdvance` has the detail.

:::caution[A particle with no horizontal course gets a fixed fallback]
The flight time is derived from the particle's horizontal velocity. When its origin and its target sit at the same horizontal position, that velocity is zero on both axes. The calculation then falls back to a fixed 9999 frames before dividing, leaving an interval far longer than any authored one.
:::
