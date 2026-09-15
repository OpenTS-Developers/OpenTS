---
key: HarvesterLoadRate
summary: Game frames between the animation ticks of a harvester taking on a load.
see_also: ["system:tiberium", "HarvesterDumpRate"]
when_omitted:
  kind: value
  value: "2"
---

Nine ticks make one lifting cycle, and a cycle removes one growth stage from the cell and adds one unit to the harvester, so the default spends 18 frames per stage. A vein harvester runs the same counter at three times the interval.

```ini title="rules.ini"
[General]
HarvesterLoadRate=1   ; one tick a frame, so a lifting cycle takes 9 frames
```
