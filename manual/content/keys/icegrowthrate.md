---
key: IceGrowthRate
summary: Minutes between passes of the ice sheet creeping out across open water.
see_also: [IceSolidifyFrameTime, IceGrowthEnabled, IceCrackingWeight]
when_omitted:
  kind: value
  value: "1"
---

```ini title="rules.ini"
[AudioVisual]
IceGrowthRate=1.5
```

Only a snow-theater scenario grows ice, and only while the map sets [`IceGrowthEnabled=yes`](/keys/icegrowthenabled/). When the delay expires, one pass runs. Every cell the map marks as growable thickens into full ice if its tile is a thin fringe piece of an existing sheet and is not itself cracked, and the neighboring tiles are re-dressed to suit. Growable is a per-cell flag stored in the map data, not a property of the tile type. The sheet therefore creeps outward one ring per pass rather than filling in at once.

The delay is counted in game frames at 900 to the minute and the fraction is discarded, so `1.5` waits 1350 frames. Setting the key to `0` stops growth outright: the pass is skipped and its timer is never restarted.

This is the spreading rate only. Cracked ice healing back to solid runs on [`IceSolidifyFrameTime`](/keys/icesolidifyframetime/) and is not paced by this figure, although both are held back by the same map flag.

:::caution[A very small or negative rate runs the pass every frame]
Anything whose product with 900 truncates to zero (below about a nine-hundredth of a minute) and any negative value alike leave the countdown reporting zero immediately. The pass then runs on every frame instead of on a delay, and each pass walks every cell of the map twice.
:::
