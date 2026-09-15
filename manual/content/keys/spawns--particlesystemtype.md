---
key: Spawns
scope: particlesystemtype
label: Particle system spawn flag
no_effect: true
see_also: [BehavesLike, SpawnFrames]
when_omitted:
  kind: value
  value: "no"
---

The name promises a switch for whether a system emits particles under its own power. What a system emits, and on what schedule, is settled by [`BehavesLike`](/keys/behaveslike/#scope-particlesystemtype) alone. A smoke or fire system emits with the flag either way, and a gas or web system lays nothing of its own with the flag either way.

No gameplay path reads the stored flag.
