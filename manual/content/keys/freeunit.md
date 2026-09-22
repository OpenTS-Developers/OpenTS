---
key: FreeUnit
summary: The vehicle, infantry or aircraft type a structure gives its owner when its buildup finishes.
see_also: ["system:production", HoverPad, PadAircraft]
when_omitted:
  kind: value
  value: "none"
---

The game looks the name up among the vehicle types, then the infantry types, then the aircraft types, so a name used by more than one kind gives the vehicle. A name that matches none of them gives nothing.

A vehicle or infantryman appears beside the structure, or on a free cell near it. If no nearby cell can take it, the owner gets the unit's written `Cost=` back instead. A [`Harvester=yes`](/keys/harvester/#scope-unittype) or [`Weeder=yes`](/keys/weeder/#scope-unittype) vehicle starts harvesting; any other unit takes its type's usual idle mission.

An aircraft appears on the structure itself, facing [`PoseDir=`](/keys/posedir/), and is set to guard. A [`HoverPad=yes`](/keys/hoverpad/) or [`Helipad=yes`](/keys/helipad/) structure docks it as if it had landed there; any other structure leaves it undocked. If the aircraft cannot be placed, the owner gets its written `Cost=` back. A structure whose `FreeUnit` is an aircraft never receives a [`PadAircraft=`](/keys/padaircraft/) aircraft, even when its own could not be placed.

A structure present when the scenario starts gives nothing, and capturing a structure gives the new owner nothing.

Whether the owner receives it also depends on what was paid. A computer house always does. A human-controlled house does only when one of two things is true of the structure:

- it was given no purchase price at all, which covers one placed by a trigger rather than produced;
- the price it paid was greater than its own price with the unit's price already deducted.

A factory charges the written price rather than the deducted one, so the second condition holds for any structure that was actually produced and whose unit is worth anything at all. The first covers one that opened without being bought and was not excluded earlier: a structure placed by a trigger during the mission. The second condition is the purchase test: it passes when the unit's price was inside what the owner actually paid, and the one input it excludes is a unit priced at nothing, which adds nothing to a factory's charge.

```ini title="rules.ini"
[MYPROC]        ; example refinery BuildingType
Cost=2000       ; already includes MYHARV
FreeUnit=MYHARV ; example harvester UnitType; an InfantryType or AircraftType also works
```

The unit's price sits inside the structure's cost rather than being added to it, but the price asked for the structure adds it straight back. Naming a free unit therefore changes neither what the structure costs to buy, nor what selling it refunds, nor what destroying it is worth. It lowers the repair bill, and it sets the figure the second condition above compares against. [`Cost=`](/keys/cost/#scope-aircrafttype) sets out the whole of what the deduction reaches.
