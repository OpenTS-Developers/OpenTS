---
key: Spawns
scope: animtype
label: Animation impact spawn
see_also: ["SpawnCount", "Bouncer", "IsMeteor"]
when_omitted:
  kind: value
  value: none
---

The animation type a thrown animation breaks into where it comes down. The children are all created at once, at the point of impact, with no creation delay. [`SpawnCount`](/keys/spawncount/#scope-animtype) fixes how many appear, and nothing appears without it.

Every thrown animation reaches the branch, whether it is a meteor or an ordinary [`Bouncer=yes`](/keys/bouncer/) one. An animation that is neither never reaches it, and neither does one that ends low over water.

A value naming no registered animation is not refused. A type of that name is created on the spot, holding no artwork and every setting at its built-in value, and the impact spawns copies of it.

:::caution[A type that spawns itself never runs out]
An animation naming its own type here, or any cycle of thrown types that leads back to it, replaces itself at every impact. At a count of `1` each impact replaces the animation on average; at `2` or more the population grows with every generation.
:::
