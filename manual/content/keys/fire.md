---
key: Fire
summary: The blast cracks the ice it lands on, and the attack cursor stops offering destroyable cliffs.
see_also: [Wall, Sparky]
when_omitted:
  kind: value
  value: "no"
---

On a snow theater the ice under the impact becomes cracked ice, and a tile that is cracked already gives way. A blast standing above a bridge deck cracks nothing. [`Wall=yes`](/keys/wall/#scope-warheadtype) cracks ice on the same path, so a wall-destroying warhead needs nothing else to do it. Despite the name, the flag sets no flame; flames belong to [`Sparky=yes`](/keys/sparky/).

:::danger[Giving way turns the cell into open water]
A blast on an already-cracked tile turns that cell to open water. A vehicle standing on it whose [`MovementZone`](/keys/movementzone/) is not one of the amphibious ones begins to sink and is stunned; an infantryman or a landed aircraft standing on it is destroyed outright. Each leaves a wake behind.
:::

```ini title="rules.ini"
[MyFlameWH] ; example WarheadType
Fire=yes
```

A cell holding a destroyable cliff normally offers the attack cursor even when nothing worth shooting stands on it. An object whose first weapon slot uses a fire warhead loses that offer. Only the first slot is read, so a second weapon with an ordinary warhead does not restore it. The cliff itself is unaffected: a fire blast that reaches the cell by any other route still has its usual chance to collapse it.
