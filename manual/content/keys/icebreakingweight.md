---
key: IceBreakingWeight
summary: Vehicle weight at or above which crossing ice breaks it open into water.
see_also: [IceCrackingWeight, Weight, IceCrackSounds]
when_omitted:
  kind: value
  value: "4"
---

```ini title="rules.ini"
[General]
IceBreakingWeight=6

[MYHVYTNK]  ; synthetic UnitType
Weight=6    ; equal to the threshold, so it breaks the ice rather than cracking it
```

The figure is compared against the crossing type's [`Weight`](/keys/weight/), a bare number set by the type definition with no unit attached. Arrival runs two weight tests in order, and the second is reached only where the first fails:

1. weight at or above this figure: the ice is broken open into water;
2. otherwise, weight at or above [`IceCrackingWeight`](/keys/icecrackingweight/): the ice is cracked;
3. otherwise, the ice is left alone.

Both comparisons are at-or-above, so a vehicle whose weight exactly matches this value takes step 1. Step 2 is reached only from a failed step 1, so a vehicle at or above this weight never cracks ice. It either opens the ice or leaves it untouched.

Only a vehicle runs the test, only in a theater whose [`IsIceGrowthEnabled`](/keys/isicegrowthenabled/) is `yes`, and only as it finishes arriving in a cell. Infantry and aircraft never trigger it however heavy they are, and a vehicle crossing on a bridge over the ice is exempt.

Breaking takes a two-by-two block of cells laid out ahead of the vehicle's facing. Every cell of that block must already be water or ice, or nothing happens at all. When the block does give way, all four cells take the broken edge tile of an ice set picked afresh for each cell, and the ice around them is re-dressed to suit. Vehicles standing on the block start sinking and are stunned. Infantry and aircraft on it are removed outright and fire the destroyed events of any trigger attached to them, and each affected object leaves a wake animation behind.

:::caution[Amphibious movement does not save the vehicle that broke through]
A vehicle standing on the block is spared the sinking when its [movement zone](/reference/enums/movement-zone/) is `Amphibious`, `AmphibiousCrusher` or `AmphibiousDestroyer`. The vehicle whose arrival broke the ice is set sinking and stunned afterwards regardless of its movement zone, so that exemption does not reach it.
:::
