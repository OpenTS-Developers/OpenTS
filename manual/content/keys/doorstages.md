---
key: DoorStages
summary: The number of frames the factory door animation steps through.
see_also: ["DoorAnim", "DamagedDoor", "DeployTime", "GateStages", "system:production"]
when_omitted:
  kind: value
  value: "0"
---

The door's completion fraction, which [`DeployTime`](/keys/deploytime/) spreads from shut to fully open, is scaled by this count to pick the frame of [`DoorAnim`](/keys/dooranim/) to draw. A closing door counts down from the count instead, and a shut door draws frame `0`. A frame at or past the count is pulled back to the last one in the range, and a negative frame is raised to `0`.

The count is the number of healthy door frames in the animation. With `DoorStages=9`, the door's travel draws frames `0` through `8`.

```ini title="art.ini"
[MYWEAP] ; example war factory, drawn from its own Image ID
DoorAnim=GAWEAP_D
DoorStages=9
```

At the default of `0` every frame resolves to `0`, so a door animation is drawn standing on its first frame throughout.

A [`DamagedDoor=yes`](/keys/damageddoor/) structure at or below [`ConditionYellow`](/keys/conditionyellow/) adds the count to the frame after that clamp, so the damaged block runs from this frame number through twice it less one and the art has to hold both blocks.
