---
key: DeleteOnStateLimit
summary: Kills a particle the moment its animation reaches the last state.
see_also: ["EndStateAI", "StartStateAI", "MaxEC"]
when_omitted:
  kind: value
  value: "no"
---

Without it, a particle that reaches [`EndStateAI`](/keys/endstateai/) is not finished. A `Gas`, `WeakGas` or `Web` particle restarts its sequence at state 0 and plays it again; a `Smoke` or `Fire` particle stops advancing and holds its last frame. In every case the particle then goes on until [`MaxEC`](/keys/maxec/) runs out, so the setting decides whether the animation or the lifetime ends the particle.

Whichever of the two comes first still ends it, so a particle whose sequence is longer than its lifetime disappears mid-animation with the flag set or not. [`Spark` and `Railgun`](/keys/behaveslike/#scope-particletype) particles never advance a state, so the flag can do nothing for them; a spark ends where it strikes something or when its lifetime expires.
