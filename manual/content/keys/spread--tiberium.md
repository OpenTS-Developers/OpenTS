---
key: Spread
scope: tiberium
label: Spread delay
when_omitted:
  kind: value
  value: "0"
  note: A delay of zero, which gives the type a spread pass on every frame.
---

The timer is reloaded with this many game frames after every spread pass, whether or not any cell spread. A scenario with [`TiberiumGrowthEnabled=no`](/keys/tiberiumgrowthenabled/) never runs the pass at all.
