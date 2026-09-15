---
key: Factory
summary: The kind of object the BuildingType produces.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: "none"
---

Only `UnitType`, `InfantryType`, `AircraftType` and `BuildingType` make the structure produce for a player. The short names `Unit`, `Infantry`, `Aircraft` and `Building` also parse. A computer house's structures pick what to build on their own without a player order, and that path accepts the short names, so a structure with one produces for the computer but never for a player. The value is matched against the engine's object-kind names without regard to case, and a value the engine does not recognize leaves the type a non-factory.

```ini title="rules.ini"
[MYWEAP] ; example war factory BuildingType
Factory=UnitType
```

The value also picks which of its house's four production slots the structure serves, so two structures naming the same kind share one slot and cannot build two objects at once. What a second one buys is the [multiple-factory speed-up](/systems/production/#more-than-one-factory). [What counts as a factory](/systems/production/#what-counts-as-a-factory) covers the other recognized names, which raise that speed-up without ever producing anything.
