---
key: RotCount
summary: Parsed rotation frame count that the engine never uses.
no_effect: true
see_also: ["Facings", "Turret"]
when_omitted:
  kind: context-dependent
  note: An AircraftType or UnitType section starts at 32 and an InfantryType section at 8. A BuildingType section is set to 32 when Turret=yes and to 1 otherwise, replacing whatever this key stored.
---

No drawing or targeting path reads the stored number. A shape-drawn vehicle takes its facing count from [`Facings`](/keys/facings/) instead, and a voxel model is turned rather than picked out of a set of frames. A BuildingType overwrites the value from [`Turret`](/keys/turret/) once the rules section has been read, so an authored figure does not even survive loading there.
