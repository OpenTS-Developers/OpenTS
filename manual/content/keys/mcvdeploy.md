---
key: MCVDeploy
summary: Parsed flag that the engine never uses.
no_effect: true
see_also: [UndeploysInto]
when_omitted:
  kind: value
  value: "no"
  note: The special options are initialized with this built-in default when the game starts.
---

The name promises the option that lets a construction yard fold back into the vehicle it came from. The flag is parsed into the scenario's special options, and no gameplay path reads it.

Undeploying is decided by the structure type instead, through [`UndeploysInto`](/keys/undeploysinto/).
