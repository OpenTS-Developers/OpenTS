---
key: ActiveAnim
summary: The animation the structure runs in its first active slot.
see_also: ["ActiveAnimDamaged", "ActiveAnimX", "ActiveAnimY", "ActiveAnimYSort", "ActiveAnimZAdjust", "ActiveAnimPowered", "ActiveAnimPoweredLight", "ActiveAnimTwo", "ActiveAnimThree", "ActiveAnimFour", "system:building-animations", "system:power"]
when_omitted:
  kind: value
  value: ""
---

The value names an animation registered in `[Animations]`. The structure runs it as an attached animation: a separate object pinned to a point on the structure's artwork, cycling on its own timing, and created and destroyed as the structure changes state. A name no `[Animations]` entry registers creates nothing.

```ini title="art.ini"
[MYPOWR] ; example power plant, drawn from its own Image ID
ActiveAnim=MYSMOKE           ; an AnimType registered in [Animations]
ActiveAnimDamaged=MYFIRE     ; used at ConditionYellow and below
ActiveAnimX=-12              ; twelve pixels left of the drawing center
ActiveAnimY=-30              ; thirty pixels up, onto the chimney
ActiveAnimZAdjust=-5         ; drawn over the structure rather than behind it
ActiveAnimPowered=no         ; keeps cycling through a power shortfall
```

A BuildingType has four active slots, and slots two, three and four repeat the same eight settings under the [`ActiveAnimTwo`](/keys/activeanimtwo/), [`ActiveAnimThree`](/keys/activeanimthree/) and [`ActiveAnimFour`](/keys/activeanimfour/) prefixes. [Building animations](/systems/building-animations/) owns what the eight do: the damaged name, the offset, the two draw-order biases, the two power flags, the fifteen-character limit on a name, and which art entry each setting is read from.

## When the slot runs

The animation is created as the structure comes online, either as construction finishes or as the scenario places it. All four slots are destroyed when the structure is sold or otherwise taken off the map. In between, an animation that plays to its end empties its slot and a looping one holds it.

Four things refill this slot once it is empty. A repair step the house can pay for starts every active slot that is not running. Installing an upgrade on a damaged structure repairs it to full strength and does the same. Power returning to the house recreates a slot left at [`ActiveAnimPowered=no`](/keys/activeanimpowered/) with [`ActiveAnimPoweredLight=yes`](/keys/activeanimpoweredlight/). And this slot alone is started again whenever a [`UnitRepair=yes`](/keys/unitrepair/) bay finishes with the vehicle it was servicing.

## What belongs to this slot alone

Two behaviors are peculiar to the first slot.

- On a [`SensorArray=yes`](/keys/sensorarray/) structure it starts two seconds (30 frames) after construction finishes. Every other path that starts it, and every path that starts the other three slots, applies no delay.
- A [`UnitRepair=yes`](/keys/unitrepair/) bay stops it as it begins repairing the vehicle standing over it, and starts it again as the visit ends.

:::caution[A repair bay's animations come back healthy]
The slot is restarted in its healthy form whenever a bay finishes with a vehicle, and [creating any animation in a given form](/systems/building-animations/#the-damaged-form) restarts every other animation the structure is running in that same form. A bay below [`ConditionYellow`](/keys/conditionyellow/) therefore drops back to healthy animations after each repair and keeps them until the structure next takes damage or a repair step runs.
:::

A type declaring four or more [`Upgrades=`](/keys/upgrades/) loses this slot to its fourth plug twice: the art entry reads the fourth plug's settings into it, and the installed plug runs its animation in it. [The upgrade slots and the active slots share one array](/systems/building-animations/#the-upgrade-slots-and-the-active-slots-share-one-array) covers what that costs.
