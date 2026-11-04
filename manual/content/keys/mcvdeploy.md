---
key: MCVDeploy
summary: Parsed flag that the engine never uses.
no_effect: true
see_also: [UndeploysInto]
when_omitted:
  kind: value
  value: "no"
  note: Each scenario starts with this built-in default.
---

Nothing in the game reads this flag, so it does not let a construction yard pack back up into a vehicle. [`UndeploysInto`](/keys/undeploysinto/) decides whether a structure can undeploy, and its page covers the extra conditions for a construction yard.
