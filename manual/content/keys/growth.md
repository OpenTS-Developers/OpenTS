---
key: Growth
summary: Game frames a Tiberium type waits between growth passes.
see_also: ["system:tiberium", "GrowthPercentage", "TiberiumGrows"]
when_omitted:
  kind: value
  value: "0"
  note: A delay of zero, which gives the type a growth pass on every frame.
---

The timer is reloaded with this many frames after every [growth pass](/systems/tiberium/#growth), whether or not any cell ripened, and `0` gives the type a pass on every frame. [`TiberiumGrows=yes`](/keys/tiberiumgrows/#scope-scenarios) multiplies the reloaded figure by `0.3`; nothing else scales it.

A scenario with [`TiberiumGrowthEnabled=no`](/keys/tiberiumgrowthenabled/) runs no pass at all, so the delay is never read there.
