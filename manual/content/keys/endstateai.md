---
key: EndStateAI
summary: The animation state that ends a particle's sequence.
see_also: ["StartStateAI", "StateAIAdvance", "DeleteOnStateLimit", "FinalDamageState"]
when_omitted:
  kind: value
  value: "0"
---

What reaching the state does depends on the [behavior](/keys/behaveslike/#scope-particletype). `Gas`, `WeakGas` and `Web` particles keep advancing until the state matches this one, and then either die (if [`DeleteOnStateLimit`](/keys/deleteonstatelimit/) is set) or restart at state 0 and go round again. `Smoke` and `Fire` particles advance only while the state is below this one, so they stop here and hold the final frame until their lifetime runs out, unless the flag kills them as they arrive. `Spark` and `Railgun` particles have no state machine and ignore the setting entirely.

A `Fire` particle also uses the figure as the stride between the four directional banks of its artwork. The frame drawn is the current state plus zero, one, two or three times this value, according to which way the flame was fired. A flame shape therefore has to hold four times this many frames, and one more if the sequence is allowed to hold its final state.

The state is held in one signed byte, so a value above 127 wraps negative. A `Smoke` or `Fire` particle then never advances at all, since its state already sits at or above the end. A `Gas`, `WeakGas` or `Web` particle wraps through the same byte and still reaches the end after that many advances. Leaving the key out puts the end at state 0. For `Gas`, `WeakGas` and `Web` the state then matches before the sequence has run at all, and such a particle dies within its first few frames when the flag is set. The figure is also what [`FinalDamageState`](/keys/finaldamagestate/) falls back to when that key is not given.
