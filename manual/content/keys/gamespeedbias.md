---
key: GameSpeedBias
summary: The multiplier folded into every house's build-time, ground-speed and air-speed figures.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: "1"
---

The value is combined into the figures a house is handed when it is assigned its difficulty. It multiplies the build-time figure, which the country's section and the difficulty section (`[Easy]`, `[Normal]`, `[Difficult]`) produce from their [`BuildTime=`](/keys/buildtime/#scope-housetype) values. It also multiplies the ground-speed and air-speed figures those sections produce. A figure above 1 therefore makes everything build more slowly while ground units and aircraft move faster. At `1.1` the build takes 10 percent longer, and the units move 10 percent faster.

This value applies in campaign games too, as do the difficulty section's own figures. Only the country's figures are dropped there.
