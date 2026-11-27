---
key: SurvivorRate
summary: The fraction of a structure's cost that decides how many survivors leave it.
see_also: ["system:capture"]
when_omitted:
  kind: value
  value: ".5"
---

A structure's survivor count is its cost multiplied by `SurvivorRate` and divided by the [`SurvivorDivisor`](/keys/survivordivisor/) that is applied for the structure's side, rounded down and then limited to between 1 and 5. The cost is what the structure's current owner pays for its type. A higher rate gives more survivors, but only within that range of 1 to 5.

Only a [`Crewed=yes`](/keys/crewed/) type produces survivors. A sold structure tries to place every survivor, unless its type sets [`UndeploysInto`](/keys/undeploysinto/). A survivor with no free spot to stand on is lost. A destroyed structure rolls for them cell by cell. [Survivors](/systems/capture/#survivors) covers those odds and how a capture or an armed demolition charge changes them.
