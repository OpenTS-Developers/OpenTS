---
key: AttackingAircraftSightRange
summary: Radius in cells revealed when a human player's aircraft fires from or at shrouded ground.
see_also: ["system:map-visibility", Sight]
when_omitted:
  kind: value
  value: "5"
---

When a human player's aircraft fires, it reveals this many cells around its own position for its owner, in place of its [`Sight=`](/keys/sight/), if any of these lies under the owner's shroud: the aircraft's own position, three points two cells from it on the diagonals, or the target's center. In a campaign, an aircraft of any player-controlled house qualifies, and the player's own shroud is tested.

The radius is a plain count of cells and is not converted from a lepton distance.

```ini title="rules.ini"
[General]
AttackingAircraftSightRange=8
```
