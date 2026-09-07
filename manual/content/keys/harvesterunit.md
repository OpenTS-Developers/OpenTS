---
key: HarvesterUnit
summary: UnitTypes the engine treats as a house's harvesters when it counts or replaces them.
see_also: ["system:tiberium", "Harvester"]
when_omitted:
  kind: value
  value: "none"
---

Every listed type counts as a harvester wherever the engine counts them: the computer's decision to queue a replacement, its judgement of whether it can still earn, the harvester census that spreads a computer house's [weighted patch search](/systems/tiberium/#finding-a-patch) across a field, the free harvester a unit crate hands a player who owns a refinery and none, and the income bookkeeping behind them. Where one harvester must be priced, queued or handed out, the engine takes the first entry the country the house acts as may own, or entry 0 when it may own none. The harvester-truce option shields every listed type from all damage but limpet warheads and keeps it out of every automatic target scan, a human house's included, and recovery from an EMP stun sends any listed vehicle back to harvesting.

The list is separate from [`Harvester=yes`](/keys/harvester/#scope-unittype): a type not named here still harvests, it is simply invisible to those counts.

An empty list leaves the computer judging that it can still earn, so it never sells its base back to buy a harvester it cannot name.
