---
key: BridgeRepairHut
summary: Turns the structure into the one an engineer walks into to rebuild a destroyed bridge.
see_also: ["system:capture"]
when_omitted:
  kind: value
  value: "no"
---

For an engineer the setting replaces both ordinary outcomes rather than adding to them, so a hut is never restored and never changes hands whoever owns it and however damaged it is. Three of its readers are the engineer's: the cursor over the structure, the cursor over the cell when the structure is only a fogged record, and [the arrival itself](/systems/capture/#repairing-a-bridge). The cursor is the repair cursor while a bridge within two cells of the hut can still be rebuilt, and the refusal cursor otherwise, so a hut whose bridge is intact offers nothing. Ownership is not tested on any of the three.

Two more readers have nothing to do with engineers. A cell holding a hut is never offered to an infantry or a vehicle as something to destroy and drive through: the move is refused outright instead. And a hut that also sets [`Immune=yes`](/keys/immune/#scope-aircrafttype) is exempted from damage before the ordinary immunity test is reached, so it survives even the forced damage that gets past `Immune=yes` everywhere else.
