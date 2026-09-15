---
key: MinProductionSpeed
summary: The floor the low-power production multiplier is held to.
see_also: ["system:power"]
when_omitted:
  kind: value
  value: ".5"
---

A house short of power builds at a multiplier taken from its power fraction, and build time is divided by that multiplier; `0.5` doubles it. This value is the floor that multiplier is raised to, so it fixes the worst production speed a shortfall can cause. Raising it to `1` removes the production penalty entirely.

At the default the floor coincides with the lowest step the ladder itself produces, so it changes nothing until it is moved. [The production ladder](/systems/power/#production) lists the steps it is applied to.
