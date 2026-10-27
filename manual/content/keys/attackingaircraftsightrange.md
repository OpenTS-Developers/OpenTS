---
key: AttackingAircraftSightRange
summary: Radius in cells revealed when a human player's aircraft fires from or at shrouded ground.
see_also: ["system:map-visibility", Sight]
when_omitted:
  kind: value
  value: "5"
---

When a human player's aircraft fires, it reveals the ground within this many cells of its position if any of these points lies under its owner's shroud:

- the aircraft's position;
- any of three points two cells from the aircraft on the diagonals;
- the center of the target.

The reveal uncovers the ground for the aircraft's owner and for the houses that [share its view](/systems/map-visibility/#whose-looks-count). It is made in addition to the aircraft's regular looks with its [sight range](/systems/map-visibility/#sight-range). In a campaign, an aircraft of any player-controlled house qualifies, and the test reads the player's own shroud.

The value counts cells, like `Sight=`. A reveal reaches at most ten cells, so a value above `10` acts as `10`, as [Sight range](/systems/map-visibility/#sight-range) explains. At `0` the aircraft reveals nothing when it fires.

```ini title="rules.ini"
[General]
AttackingAircraftSightRange=8
```
