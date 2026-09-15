---
key: Official
scope: scenarios
label: Unused copy
when_omitted:
  kind: value
  value: "no"
---

The entry is read at the very start of the scenario load, before the theater is chosen or the rules are prepared. In a multiplayer or skirmish game its value goes straight to the [start-position pass](/keys/official/#scope-scenarios-2), which settles where every house starts before the map's objects are read.

The same assignment is read a second time near the end of the load, when the multiplayer fixups create the starting units. That copy reaches the same start-position logic with the same value.
