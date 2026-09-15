---
key: LargeVisceroid
scope: unittype
label: Large visceroid behavior
see_also: ["SmallVisceroid", "AltImage", "NonVehicle"]
when_omitted:
  kind: value
  value: "no"
---

The flag puts the vehicle on the same creature conduct that [`SmallVisceroid=yes`](/keys/smallvisceroid/#scope-unittype) describes: the aimless wandering, the run for Tiberium below [`ConditionYellow`](/keys/conditionyellow/) health, and the attack frames from [`AltImage`](/keys/altimage/). It also shares the exemptions from EM pulses, immobilization, turret facing, jellyfish stings and shadows, and it forces [`NonVehicle`](/keys/nonvehicle/) on in the same way.

What it does not have is merging. A large visceroid neither summons a neighbor nor is summoned by one, and two of them standing beside each other block one another's cells like any other pair of vehicles. Marking a type this way is therefore the whole of the difference: whether it grows by absorbing its own kind, or is what that absorption produces.

A type is not obliged to be reachable through merging to set the flag. The UnitType a merge actually produces is the one named by [`LargeVisceroid`](/keys/largevisceroid/#scope-global-rules) in the global rules, which is a separate choice from this one. A type with this flag that nothing names there simply behaves as a large visceroid wherever it is placed.
