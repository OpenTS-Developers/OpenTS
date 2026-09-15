---
key: DropPodHeight
summary: Initial drop-pod altitude above the landing cell, measured in leptons.
see_also: [DropPodAngle, DropPodSpeed, "system:drop-pods"]
---

The engine adds this value to the terrain height at the destination to fix the altitude the pod starts its fall from. Together with [`DropPodAngle`](/keys/droppodangle/), it also determines the horizontal distance between the starting point and the landing cell.

Larger values lengthen the visible descent and, because falling speed increases with height, change the speed profile as well as the starting altitude.

```ini title="rules.ini"
[General]
DropPodHeight=1500
```
