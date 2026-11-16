---
key: AmbientChangeRate
summary: Minutes between one step of the ambient light fade and the next.
see_also: [AmbientChangeStep, Ambient, IonAmbient, "system:ion-storms"]
when_omitted:
  kind: value
  value: ".2"
---

The value is in game minutes of 900 frames, so `.2` waits 180 frames between steps. Together with [`AmbientChangeStep`](/keys/ambientchangestep/), it sets how long the map takes to reach a new ambient level, whether a trigger action or [an ion storm](/systems/ion-storms/#the-ambient-ramp) changed the target.

The first step comes as soon as the target changes, unless the wait after an earlier step is still running. The wait only spaces the steps that follow.

:::caution[Keep AmbientChangeRate above 0]
At `0` the ambient level never changes again. A scripted lighting change and a storm's darkening both set a new target that never shows on the map.
:::

The [Set ambient rate...](/mapping/actions/taction-set-ambient-rate/) trigger action changes this value, and a saved game keeps the change. The next scenario starts again from the value its rules files and map give.
