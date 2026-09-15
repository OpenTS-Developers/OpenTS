---
key: PreProductionAnim
summary: The animation a structure runs while work is being set up on it.
see_also: ["PreProductionAnimDamaged", "PreProductionAnimX", "PreProductionAnimY", "PreProductionAnimYSort", "PreProductionAnimZAdjust", "ProductionAnim", "ActiveAnim", "system:production"]
when_omitted:
  kind: value
  value: ""
---

The slot holds an animation registered in `[Animations]` and runs it on the terms [Building animations](/systems/building-animations/) covers. Two structures use it. On the construction yard it is the lead-in that [`ProductionAnim`](/keys/productionanim/) takes over from; on the refinery the two are separate slots and neither ends the other.

- A [`ConstructionYard=yes`](/keys/constructionyard/) structure starts it when a structure it built reports that construction has begun, choosing between the two names from its own health. It is stopped when contact with that structure drops, and replaced by the production slot when the structure reports its buildup finished.
- A [`Harvester=yes`](/keys/harvester/#scope-unittype) vehicle starts it on the structure it is docked with as it begins to unload, always in the healthy form. Nothing stops it, so an animation that plays to its end empties the slot and a looping one holds it for the rest of the structure's life. The production slot is a separate slot and does not stop it, and nothing on a refinery ever does. The two animations run together from then on.

:::caution[The harvester starts the slot on whatever stands west of it]
The vehicle takes the structure one cell west of its own cell and starts that structure's pre-production slot without testing what kind of structure it is. Only the later start of the production slot is limited to a [`Refinery=yes`](/keys/refinery/) structure.
:::

The two names are read from the structure's `[<Image ID>]` art entry, and the offset and the two draw-order biases from the entry named after the BuildingType itself. [Where the settings are read](/keys/productionanim/#where-the-settings-are-read) covers that split. The offset and biases are read only once the slot holds a name, and the slot has no power flags.
