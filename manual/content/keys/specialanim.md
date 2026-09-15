---
key: SpecialAnim
summary: The animation the structure runs in its first special slot.
see_also: ["SpecialAnimDamaged", "SpecialAnimX", "SpecialAnimY", "SpecialAnimYSort", "SpecialAnimZAdjust", "SpecialAnimPowered", "SpecialAnimPoweredLight", "SpecialAnimTwo", "SpecialAnimThree", "ActiveAnim", "UnitRepair", "SiloDamage", "FirestormWall", "system:building-animations"]
when_omitted:
  kind: value
  value: ""
---

The value names an animation registered in `[Animations]`. The structure runs it as an attached animation: a separate object pinned to a point on the structure's artwork, cycling on its own timing, and created and destroyed as the structure changes state. A name no `[Animations]` entry registers creates nothing. [Building animations](/systems/building-animations/) covers what the seven companion settings do and which art entry each of them is read from.

```ini title="art.ini"
[MYDEPOT] ; example service depot, UnitRepair=yes and no Image= in rules.ini
SpecialAnim=MYDEPTC1         ; runs as the repair begins
SpecialAnimTwo=MYDEPTC2      ; takes over once MYDEPTC1 plays out
SpecialAnimThree=MYDEPTC3    ; runs as the customer is released
SpecialAnimZAdjust=-100      ; all three drawn over the structure
SpecialAnimTwoZAdjust=-100
SpecialAnimThreeZAdjust=-100
```

## What starts a special animation

An active slot is filled as the structure comes online and holds its animation for as long as the structure stands. A special slot is filled only by an event, and only three flags on the structure's type raise those events: [`UnitRepair=yes`](/keys/unitrepair/), [`SiloDamage=yes`](/keys/silodamage/) and [`FirestormWall=yes`](/keys/firestormwall/). A structure with none of the three stores every one of the twenty-four settings the three slots accept and, apart from the one power route below, creates nothing from them.

The three slots are not interchangeable, and which of them a structure can use follows from which flag it has.

### A service depot

On a `UnitRepair=yes` depot the three slots are one sequence around [the repair cycle](/systems/repair/#one-step-at-a-time):

- The **first** slot is created as the depot begins repairing its customer, together with the structure's [`ProductionAnim`](/keys/productionanim/), and the first active slot is stopped at the same moment.
- The **second** slot is created when the first slot's animation plays to its end, and only while the depot still holds the customer on the repair mission.
- The **third** slot is created as the visit ends: because the customer left, because the repair finished, or because the house could not pay for the next step. The second slot and `ProductionAnim` are stopped at that moment and the first active slot starts again. A depot that drops the repair mission with either still running is caught on its next update, which starts the third slot and stops the other two but leaves the first active slot dark.

:::caution[A looping first animation stalls the sequence]
The second slot waits for the first one to play out, and no part of the repair cycle stops the first. An animation that loops therefore holds the first slot for the rest of the structure's life: the second slot never appears, and the first keeps running between visits as well as during them.
:::

### A storage structure

On a `SiloDamage=yes` structure the first slot alone is the fill indicator, and its frame is set directly from the stored amount rather than advancing on its own. [`SiloDamage`](/keys/silodamage/) covers that arithmetic and which frames are drawn. The fill indicator never reaches the second or third slot.

:::danger[A fill indicator with no animation crashes the game]
The frame is written to the first slot without checking that anything is in it. A `SiloDamage=yes` structure whose stored amount first reaches an eighth of its [`Storage=`](/keys/storage/) crashes the game if `SpecialAnim=` is missing, or names an animation no `[Animations]` entry registers. A [`SpecialAnimDamaged=`](/keys/specialanimdamaged/) on its own does not cover the gap, because the indicator is always started in its healthy form.
:::

### A firestorm wall section

A `FirestormWall=yes` section takes the first two slots for animations named in `[AudioVisual]`, at fixed offsets and a fixed depth bias of -10. [The firestorm wall](/systems/laser-fences/#raising-and-lowering-the-wall) covers when each of the two appears. Neither goes through the slot mechanism, so the section's own `SpecialAnim=` and [`SpecialAnimTwo=`](/keys/specialanimtwo/), their offsets and their biases are all read and then bypassed. The wall itself never reaches the third slot.

### A powered light on any structure

The pass that brings a house's structures into line with its power supply reaches all three special slots as it reaches the active ones. A slot left at [`SpecialAnimPowered=no`](/keys/specialanimpowered/) with [`SpecialAnimPoweredLight=yes`](/keys/specialanimpoweredlight/) is therefore created whenever the house's power reaches full, whatever the structure is — the one route by which a structure with none of the three flags runs a special animation. [Power](/systems/building-animations/#power) covers what the two flags do and which of them is read.
