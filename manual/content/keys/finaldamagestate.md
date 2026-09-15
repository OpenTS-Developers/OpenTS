---
key: FinalDamageState
summary: The last animation state at which a flame particle still burns what it drifts over.
see_also: ["EndStateAI", "MaxDC", "Damage", "Normalized"]
when_omitted:
  kind: inherited
  note: The value EndStateAI holds in the same section.
---

A [`Fire`](/keys/behaveslike/#scope-particletype) particle checks its state against this one before it applies [`Damage`](/keys/damage/#scope-particletype), so a flame that has burned past the state named here drifts on and scorches nothing. Since a state only ever climbs, the check is one-way: once a flame is past it, it never damages again. `Gas` and `Web` particles run their own damage paths and never read it.

A flame marked [`Normalized=yes`](/keys/normalized/#scope-particletype) also uses the figure to pace itself, dividing its flight time by one more than this value. A value at or above the state the flame reaches leaves the sequence damaging for its whole length.

:::danger[A normalized flame given -1 stops the game]
A flame marked `Normalized=yes` divides its flight time by one more than this figure, so `-1` divides by zero. The result is stored as an interval of `0` frames. Half the flames of the type (those whose creation order makes no adjustment to it) then divide by zero on their first logic frame and the game stops, exactly as [`StateAIAdvance=0`](/keys/stateaiadvance/) does.
:::

The default is not a fixed number but whatever [`EndStateAI`](/keys/endstateai/) holds in the same section, taken as that key is read. A layer that changes the end state and does not repeat this key moves both together. A layer that reopens the section for any other reason and omits both keys returns this one to the end state, discarding a shorter damage window set earlier.
