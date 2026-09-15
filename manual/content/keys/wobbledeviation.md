---
key: WobbleDeviation
summary: How far a hovering jumpjet unit drifts above and below its flight level.
see_also: [WobblesPerSecond, CruiseHeight, Climb]
when_omitted:
  kind: value
  value: "40"
---

The figure is the amplitude of the drift, in leptons, with 256 to a cell. The height a jumpjet aims for each frame is its flight level plus this figure scaled by a sine wave. The unit rides between one deviation above the level and one below it. [`WobblesPerSecond`](/keys/wobblespersecond/) sets how quickly it works through that wave, and [`Climb`](/keys/climb/) caps how fast it can actually follow it.

Only a hovering or cruising jumpjet wobbles. Climbing out, coming down and sitting on the ground all reset the wave to its start. The drift appears the moment the unit levels off and stops the moment it begins its descent.

```ini title="rules.ini"
[JumpjetControls]
WobbleDeviation=40
```

The drift is not purely cosmetic. While a jumpjet is still short of the cell it is heading for, the locomotor throttles it back by a tenth whenever it is flying below half the height it wants. Below a quarter of that height it throttles back by a further tenth. Both thresholds are fractions of the wobbling target rather than of the flight level. A large deviation therefore makes a cruising jumpjet lose and regain a little ground speed in time with its bobbing.
