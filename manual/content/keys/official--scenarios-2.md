---
key: Official
scope: scenarios-2
label: Starting-point selection
when_omitted:
  kind: value
  value: "no"
---

```ini title="map file"
[Basic]
Official=yes
```

In a multiplayer or skirmish game, and not on a generated random map, this assignment decides which of the placed waypoints `0` through `7` the [start positions](/systems/starting-forces/#the-start-position) are drawn from. It does so only when no seat in the launch file named a position by number. Those waypoints are the pool each house draws from: the first house takes one at random, and every house after it takes whichever open one lies furthest from those already held.

Marked official, the pass draws from waypoint `0` onward up to a cutoff. The cutoff is the larger of two numbers: how many houses have to be seated, and how many waypoints are placed in an unbroken run starting at `0`. A placed waypoint beyond the cutoff is not drawn from. Left at the default, every placed waypoint is drawn from. A seat that named a position makes every placed waypoint eligible whatever this assignment says.

A house left over once the eligible waypoints have all been taken starts on a random cell of open ground, found as the scenario finishes loading.

:::danger[A map that cannot host a starting position hangs the load]
Each open-ground position is searched for until the search succeeds. On a map where no cell can take a starting position the search never ends and the scenario never finishes loading.
:::
