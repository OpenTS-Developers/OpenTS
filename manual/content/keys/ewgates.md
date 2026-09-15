---
key: EWGates
summary: The gates a computer house fits into the east-west runs of its base perimeter, in order of preference.
see_also: [NSGates, "system:ai-base-building"]
when_omitted:
  kind: value
  value: ""
---

The [perimeter planner](/systems/ai-base-building/#walls-and-gates) fits the first entry [the country this house acts as](/keys/actslike/) [may own](/keys/owner/) into a wall run along the north or south edge of the base. Wall runs along those two edges travel east to west. The east and west edges take [`NSGates`](/keys/nsgates/) instead. Nothing requires the entry to be a gate.

If the list holds no entry the country may own, the planner finds nothing and still builds a gate node from the result.
