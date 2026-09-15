---
key: IceCrackingWeight
summary: Vehicle weight at or above which crossing ice cracks it.
see_also: [IceBreakingWeight, Weight, IceCrackSounds, IceSolidifyFrameTime]
when_omitted:
  kind: value
  value: "2"
---

The figure is compared against the crossing type's [`Weight`](/keys/weight/), a bare number set by the type definition with no unit attached. The comparison is at-or-above. The [`IceBreakingWeight`](/keys/icebreakingweight/) test is tried first, so the band that cracks ice runs from this value up to, but not including, the breaking weight. Setting this value above the breaking weight leaves the band empty, and no vehicle then cracks ice at all. Only a vehicle runs the test, only in a theater whose [`IsIceGrowthEnabled`](/keys/isicegrowthenabled/) is `yes`, only as it finishes arriving in a cell, and never while it is on a bridge over the ice.

Cracking replaces the cell's tile with the cracked tile of one of the three ice sets, picked at random. It then re-dresses the ice around it, plays one of [`IceCrackSounds`](/keys/icecracksounds/), and schedules the cell to refreeze [`IceSolidifyFrameTime`](/keys/icesolidifyframetime/) frames later. The vehicle drives on unharmed. A cell that is not ice at all is left alone.

Crossing a cell whose ice is already cracked does not crack it again. That case falls through to breaking, so a vehicle in this weight band that recrosses a cell before it has refrozen takes the breaking path instead. Weapon fire cracks ice on its own account as well, with no reference to weight. Any warhead that destroys walls or sets fires cracks the ice at the cell it lands on.
