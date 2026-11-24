---
key: AmbientChangeStep
summary: How far the ambient light moves on each step of a fade, as a fraction of full ambient light.
see_also: [AmbientChangeRate, Ambient, IonAmbient, "system:ion-storms"]
when_omitted:
  kind: value
  value: ".1"
---

Each step moves the ambient level toward its target by this fraction of full light. The step is converted to the hundredths that [`Ambient`](/keys/ambient/) is stored in, so `.1` moves the level by 10 hundredths. A step never passes the target, so the last step of a fade can be smaller.

The conversion truncates to whole hundredths. Floating-point rounding can take a value one hundredth lower, so `.29` moves the level by 28 hundredths.

:::caution[Keep AmbientChangeStep at .01 or above]
A smaller value truncates to a step of 0, so the ambient level never reaches a new target. A negative value moves the level away from its target.
:::

The [Ambient light <= ...](/mapping/events/tevent-ambient-less-than/) and [Ambient light >= ...](/mapping/events/tevent-ambient-greater-than/) trigger events compare the current ambient level, which moves one step at a time. A trigger that waits for a new target level fires only when the fade reaches that level.

The [Set ambient step...](/mapping/actions/taction-set-ambient-step/) trigger action changes this value, and a saved game keeps the change. The next scenario starts again from the value its rules files and map give.
