---
key: AtmosphereEntry
summary: The animation drawn high above the map where a drop pod enters the scenario.
see_also: [DropPod, DropPodHeight, DropPodAngle, "system:drop-pods"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
AtmosphereEntry=MYPODRING ; an AnimType registered in [Animations]
```

A pod starts its run from an elevated coordinate offset from the cell it is aimed at, and this animation is created there on the frame the passenger is first placed. It appears once per pod, before any part of the descent begins. [Approach selection](/systems/drop-pods/#approach-selection) covers where that coordinate falls for each of the four approaches, and nothing else in the game reads the setting.

A placement that fails is retried at once, and the retry does not create the effect. A pod aimed somewhere it cannot start from therefore arrives without one.

:::danger[An unset animation crashes the game as the first pod arrives]
The placement creates the animation without first checking that one was named, so a rules layer that leaves the key unset crashes the game the moment a [Drop Pods superweapon or a drop-pod team](/systems/drop-pods/#entry-paths) sends its first passenger.
:::
