---
key: SmallVisceroid
scope: unittype
label: Small visceroid behavior
see_also: ["LargeVisceroid", "AltImage", "NonVehicle", "system:tiberium"]
when_omitted:
  kind: value
  value: "no"
---

The flag puts the vehicle on the creature's conduct instead of a vehicle's. Standing still on a guard or empty mission with no destination and no target, it picks one of the eight directions and steps into that cell when it can enter it. It keeps the last direction two times in three. Below [`ConditionYellow`](/keys/conditionyellow/) health it heads instead for Tiberium within a sixteen-cell search radius, and once it is standing on Tiberium it accepts only another Tiberium cell as its next step. Standing there mends nothing on its own: the repair steps run only for a type that also sets [`TiberiumHeal=yes`](/keys/tiberiumheal/), or that holds the `TIBERIUM_HEAL` ability from [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/). This flag supplies the steering alone.

Merging is what separates it from a [`LargeVisceroid=yes`](/keys/largevisceroid/#scope-unittype) type. Standing still, it looks at the eight cells around it from north clockwise and stops at the first that holds another small visceroid. If that one has no destination and no target of its own, it is sent here. Finding a neighbor ends the pass either way, so a visceroid with one beside it neither wanders nor makes for Tiberium that frame. Two small visceroids never block each other's cell, so the one that was sent drives all the way onto this one. On arrival the visceroid it was sent to is turned into the UnitType named by [`LargeVisceroid`](/keys/largevisceroid/#scope-global-rules) at that type's full strength, while the arriving one deletes itself.

Both visceroid flags have the same handful of exemptions. [`NonVehicle`](/keys/nonvehicle/) is forced on after this key is read, whatever the section said. An EM pulse passes over the creature without stunning it, and being immobilized does not stop it firing, which no other object may do. It is not required to bring a turret to bear before it shoots, a [`Jellyfish=yes`](/keys/jellyfish/) unit never stings it, and it is drawn without a shadow. While wandering it is drawn from its ordinary artwork; for the five frames of an attack it is drawn from [`AltImage`](/keys/altimage/).
