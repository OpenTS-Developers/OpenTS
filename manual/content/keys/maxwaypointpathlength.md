---
key: MaxWaypointPathLength
summary: Number of markers a player may place into one waypoint path.
when_omitted:
  kind: value
  value: "15"
---

A path already holding this many markers takes no more. The cursor over an empty cell turns into the refusal cursor, and placing the marker that reaches the count also drops the player out of [Waypoint Mode](/commands/waypointmode/). The same limit closes the click that loops the selected path.

The count is not the only thing that closes a path: one that has been looped refuses further markers whatever its length. [Waypoint paths](/systems/waypoint-paths/) covers plotting and editing.
