---
key: DropPodSpeed
summary: Minimum per-frame movement speed for a descending drop pod.
see_also: [DropPodAngle, DropPodHeight, "system:drop-pods"]
---

Each frame the engine computes a falling speed from the pod's height above the ground, and uses this value instead when the computed speed is lower. A pod therefore begins faster when it is high above the map, then slows until it reaches the configured floor. The exact per-frame formula is on the [Drop pods system page](/systems/drop-pods/#descent-and-airborne-effects).

[`DropPodAngle`](/keys/droppodangle/) splits that movement between the horizontal approach and the vertical descent. This is not a fixed vertical speed.
