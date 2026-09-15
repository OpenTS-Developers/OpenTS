---
key: VeinholeGrowthRate
summary: Game frames between one veinhole monster's growth steps.
see_also: ["system:veins", "VeinholeShrinkRate", "MaxVeinholeGrowth"]
when_omitted:
  kind: value
  value: "100"
---

A monster's first step falls due exactly this many frames after it is created. Every step after that reloads the timer with this figure plus a random extra of up to half of it. Steps therefore land between this interval and one and a half times it. One step covers 1 to 5 cells, and the [growth step](/systems/veins/#growth) itself decides whether any of them can be taken. No cell is taken at all while the scenario's [`VeinGrowthEnabled`](/keys/veingrowthenabled/) switch is off. A short interval spreads a field faster only while there is room under [`MaxVeinholeGrowth`](/keys/maxveinholegrowth/) and ground the field accepts.

```ini title="rules.ini"
[General]
VeinholeGrowthRate=300  ; a step every 300 to 450 frames, or 20 to 30 seconds
```
