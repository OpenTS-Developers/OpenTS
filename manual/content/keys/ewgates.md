---
key: EWGates
summary: The gates a computer house fits into the east-west runs of its base perimeter, in order of preference.
see_also: [NSGates, "system:ai-base-building"]
when_omitted:
  kind: value
  value: ""
---

The first entry the country this house acts as may own is the gate the [perimeter planner](/systems/ai-base-building/#walls-and-gates) fits into a wall run along the north or south edge of the base — the two edges whose runs travel east to west. The east and west edges take [`NSGates`](/keys/nsgates/) instead. Nothing requires the entry to be a gate.

The engine builds a gate node from that entry without checking that the list held one that country may own.
