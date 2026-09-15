---
key: IceSolidifyFrameTime
summary: Frames a cracked ice cell waits before it freezes back to solid.
see_also: [IceGrowthRate, IceGrowthEnabled, IceCrackingWeight, IceBreakingWeight, IceCrackSounds]
when_omitted:
  kind: value
  value: "500"
  note: A little over 33 seconds at 15 frames to the second.
---

```ini title="rules.ini"
[AudioVisual]
IceSolidifyFrameTime=1000
```

Ice cracks in two ways: under a vehicle whose [`Weight`](/keys/weight/) reaches [`IceCrackingWeight`](/keys/icecrackingweight/) but stays below [`IceBreakingWeight`](/keys/icebreakingweight/), and under a warhead that is [`Fire=yes`](/keys/fire/) or [`Wall=yes`](/keys/wall/#scope-warheadtype) going off on it. The cell notes the frame it may heal on, which is the current frame plus this figure, and the healing pass restores it on the first frame past that. Fifteen frames make a second.

Healing takes neighbors with it: as a cell freezes, any cracked cell directly north, east, south or west of it is restored at the same moment whatever its own timer says. A long crack left by a column of vehicles therefore closes in patches rather than in the order it was made.

While the ice is cracked it will not thicken further, and a second heavy crossing of the same cell breaks through it instead of cracking it again.

:::caution[Cracks never heal on a map with ice growth switched off]
The healing pass runs only while the map sets [`IceGrowthEnabled=yes`](/keys/icegrowthenabled/); the cracking itself does not check the flag. On a snow map with growth disabled, every crack is permanent for the rest of the scenario and the next heavy vehicle over one goes straight through.
:::
