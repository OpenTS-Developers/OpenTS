---
key: Radar
summary: Whether the structure can supply the player's radar map.
see_also: ["system:power"]
when_omitted:
  kind: value
  value: "no"
---

The radar map is raised for the local player's house when all of these hold:

- No ion storm is running.
- The house's power output is at least its drain.
- It owns a structure of a `Radar=yes` type that is switched on, out of limbo, on the map and not being deconstructed.

A scenario set to [`FreeRadar=yes`](/keys/freeradar/) skips that search but still has to pass the storm and power tests.

:::caution[The search stops at the first radar it finds]
Buildings are scanned in creation order and the scan ends at the first eligible structure, which supplies the radar only if it is not stunned. A stunned radar found first therefore keeps the map dark even when a second, working one stands beside it.
:::

The value also marks the structure as an intelligence target. Once a house has a spy inside an enemy's `Radar=yes` structure, every look that enemy's objects make also uncovers the map for the spy's house.
