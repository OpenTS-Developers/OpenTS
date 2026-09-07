---
key: NSGates
summary: The gates a computer house fits into the north-south runs of its base perimeter, in order of preference.
see_also: [EWGates, "system:ai-base-building"]
when_omitted:
  kind: value
  value: ""
---

The first entry the country this house acts as may own is the gate the [perimeter planner](/systems/ai-base-building/#walls-and-gates) fits into a wall run along the east or west edge of the base — the two edges whose runs travel north to south. The north and south edges take [`EWGates`](/keys/ewgates/) instead. Nothing requires the entry to be a gate.

The engine builds a gate node from that entry without checking that the list held one that country may own.
