---
key: DeployTime
summary: How long a gate or a factory door takes to open or close, in game minutes.
see_also: ["system:walls-and-gates", "Gate"]
when_omitted:
  kind: value
  value: "0"
---

The figure counts game minutes and takes a fraction. One game minute is 900 frames, so `.044` is about 40 frames of travel and `1` would be a full 900. The stock gates ship at `.044`.

```ini title="rules.ini"
[GAGATE_A]
Gate=yes
DeployTime=.044   ; roughly 40 frames of travel each way
```

The same figure times the opening and the closing, and two structures show it:

| Structure | What the figure times |
| --- | --- |
| A [`Gate=yes`](/keys/gate/) BuildingType | The [gate's travel](/systems/walls-and-gates/#holding-and-closing) between shut and open. How far along that travel is picks which of the [`GateStages`](/keys/gatestages/) frames to draw. |
| A [`WeaponsFactory=yes`](/keys/weaponsfactory/) BuildingType | The door animation played while a finished vehicle drives out. How far along it is picks which frame of [`DoorAnim`](/keys/dooranim/) to draw, out of [`DoorStages`](/keys/doorstages/). |

Both frame counts are set in `art.ini`, which the example above leaves alone.

At `0` the movement reaches its end position on the first pass. A gate then jumps between shut and open without drawing an intermediate frame, and a factory door stands on its first frame throughout.

Despite the name, the figure has no part in an MCV deploying into a structure, which is [`DeploysInto`](/keys/deploysinto/), nor in the automatic deployment that [`MCVDeploy`](/keys/mcvdeploy/) controls.

:::caution[Only a structure does anything with the figure]
Setting it on a transport aircraft, a transport vehicle or an infantry type changes nothing a player can see.

A transport aircraft starts the count when a passenger comes aboard and stops it once it is full, but nothing that draws or moves the aircraft ever asks how far the count has got.

A vehicle is only ever asked to end the count, never to start it, and ending a count that never began is refused. A transport vehicle therefore loads and unloads with no delay at all, whatever the figure says. An InfantryType stores the figure and does nothing further with it.
:::
