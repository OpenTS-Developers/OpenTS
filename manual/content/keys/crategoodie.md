---
key: CrateGoodie
summary: Whether the vehicle type is in the random pool a unit crate draws from.
see_also: ["system:crates"]
when_omitted:
  kind: value
  value: "no"
---

The draw is reached only when [`UnitCrateType`](/keys/unitcratetype/) names nothing and the crate has not already been spent on a free unit ([crates](/systems/crates/)). There are two: the MCV handed to a house that has lost every building but can still afford to rebuild, and the harvester handed to a house that owns a refinery but no harvesters. The pick then chooses registered UnitTypes at random until one qualifies: the type sets this flag, it is ownable by the collector's house, and a type in the [`BaseUnit`](/keys/baseunit/) list qualifies only when the match has bases enabled.

The engine first checks that the pool is not empty. If no registered type qualifies for the collecting house, the draw is skipped and nothing is produced. When at least one type qualifies, the random pick repeats without an attempt limit until it lands on one.
