---
key: Trainable
summary: Allows an object of this type to accumulate experience from kills and receive veterancy-crate promotions.
see_also: ["system:veterancy"]
when_omitted:
  kind: context-dependent
  note: "`yes` for AircraftTypes, InfantryTypes, and UnitTypes; BuildingTypes start at `no` and must set the key to earn from kills or receive a veterancy-crate promotion."
---

The kill test is made on the type that dealt the fatal damage, at the moment the victim dies. A type set to `no` never accumulates anything, however much it destroys.

The key also gates the radius sweep of a veterancy crate. An object of a `Trainable=no` type is skipped without losing any rank it already holds. The crate still ignores ownership, so eligible enemy and neutral objects are promoted alongside the collector's own.

Other ways of assigning a rank ignore this key. An object promoted by deploying from a promoted vehicle, a reinforcement's [`VeteranLevel`](/keys/veteranlevel/), or an armory holds that rank and everything it unlocks regardless of this setting.
