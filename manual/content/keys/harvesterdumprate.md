---
key: HarvesterDumpRate
summary: Minutes a docked harvester spends handing over each stored unit.
see_also: ["system:tiberium", "HarvesterLoadRate", "Storage"]
when_omitted:
  kind: value
  value: ".016"
---

The dumping counter advances one step per frame, and it releases one stored unit each time it reaches this many minutes' worth of steps. At 900 frames to the minute the default moves one unit roughly every 15 frames, so a full harvester takes that long for every unit it carries.

```ini title="rules.ini"
[General]
HarvesterDumpRate=.03   ; one unit every 27 frames
```
