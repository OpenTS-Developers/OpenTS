---
key: HarvesterUnit
summary: UnitTypes the engine treats as a house's harvesters when it counts or replaces them.
see_also: ["system:tiberium", "Harvester"]
when_omitted:
  kind: value
  value: "none"
---

```ini title="rules.ini"
[General]
HarvesterUnit=HARV
```

The value is a comma-separated list of UnitType IDs. Every listed type counts as a harvester wherever the engine counts them:

- the computer's decision to queue a replacement, and its judgement of whether it can still earn;
- the census behind a computer house's [weighted patch search](/systems/tiberium/#finding-a-patch);
- the free harvester a unit crate hands a player who owns a refinery and none;
- the income bookkeeping behind those counts.

Where one harvester must be priced, queued or handed out, the engine takes the first entry the country the house [acts as](/keys/actslike/) may own, or entry 0 when it may own none.

Every listed type is also the set the [harvester truce](/keys/harvesterimmune/) shields from damage and from automatic target scans, even when a human house is the one scanning. Recovery from an EMP stun is unrelated to the truce, and sends any listed vehicle back to harvesting.

The list is separate from [`Harvester=yes`](/keys/harvester/#scope-unittype): a type not named here still harvests, it is simply invisible to those counts.

An empty list leaves the computer judging that it can still earn, so it never sells its base back to buy a harvester it cannot name.
