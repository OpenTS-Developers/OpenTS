---
key: BuildSpeed
summary: The multiplier that turns an object's cost into its base build time.
when_omitted:
  kind: value
  value: "1"
---

Base build time in game frames is the object's [`Cost`](/keys/cost/#scope-aircrafttype) multiplied by this value and by nine tenths of a frame per credit. At the default, an object costing 1000 takes one minute to build. The shipped `rules.ini` sets `.8`, so the same object takes 48 seconds.

Everything else that shapes production is applied to that figure afterwards: the country and difficulty [`BuildTime`](/keys/buildtime/) multipliers, the [game-speed bias](/keys/gamespeedbias/), the [power](/systems/power/#production) divisor, the count of factories, and the [wall coefficient](/keys/wallbuildspeedcoefficient/).

The value applies to every producible object type, so raising it lengthens every build in the game rather than any one category. A value of `2` doubles every base build time.
