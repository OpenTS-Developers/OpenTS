---
key: NSGates
summary: The gates a computer house fits into the north-south runs of its base perimeter, in order of preference.
see_also: [EWGates, "system:ai-base-building"]
when_omitted:
  kind: value
  value: ""
---

The [perimeter planner](/systems/ai-base-building/#walls-and-gates) fits the first entry [the country this house acts as](/keys/actslike/) [may own](/keys/owner/) into a wall run along the east or west edge of the base. Wall runs along those two edges travel north to south. The north and south edges take [`EWGates`](/keys/ewgates/) instead. Nothing requires the entry to be a gate.

If the list holds no entry the country may own, the planner finds nothing and still builds a gate node from the result.
