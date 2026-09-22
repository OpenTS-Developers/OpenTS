---
key: Value
summary: Credits paid for each growth stage of a Tiberium type.
see_also: ["system:tiberium"]
when_omitted:
  kind: value
  value: "0"
  note: Cells of the type are worth nothing.
---

The [harvester's patch search](/systems/tiberium/#finding-a-patch) ranks cells by this value multiplied by their growth stage plus one, so a full-grown cell counts twelve times the setting.

A harvester stores growth stages rather than credits. For most houses a stored unit is converted at this value only when the house spends it, so changing the setting re-prices Tiberium already sitting in a silo. A computer house in a skirmish or multiplayer game is the exception. It converts each unit as it is handed over and never stores it, so a later change to the figure leaves that house's income unchanged.
