---
key: AltHomeCell
summary: The waypoint the view opens on instead, when the first global flag is set.
see_also: [HomeCell]
when_omitted:
  kind: value
  value: "98"
---

```ini title="map file"
[Basic]
HomeCell=98
AltHomeCell=42
```

This is the branch a mission takes when a campaign wants the player to start looking somewhere else because of what happened earlier. The value is a waypoint number, read exactly as [`HomeCell`](/keys/homecell/) is. It is chosen over that key whenever global flag `0` is set as the map is opened. Because the flag is carried forward from the previous mission, the choice is settled before the player can do anything about it.

The keys do not share a fallback. The repair that supplies a cell for an unplaced waypoint is applied to [`HomeCell`](/keys/homecell/) alone. An alternate the scenario never placed still holds the waypoint table's empty marker, cell `0,0`, and the view is centered on that corner of the map. The out-of-range hazard described under [`HomeCell`](/keys/homecell/) reaches the alternate too, without even that repair in front of it.

:::caution[A multiplayer game overwrites the alternate]
Once the starting position of a multiplayer or skirmish game has been computed from the player's own objects, the alternate is set equal to the ordinary home waypoint, so whatever the map wrote here is discarded.
:::
