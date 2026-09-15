---
title: Delete waypoints from the end of the selected path
category: feature
release: 0.2.0
targets:
- type: command
  id: DeleteWaypoint
  effect: changed
credit: [ZivDero, dkeeton]
---

Delete Waypoint with no waypoint picked up now removes the last waypoint of the selected path, so pressing it repeatedly takes the path apart from the end; before, it did nothing unless a waypoint was held. A path's loop now survives a deletion, where before any removal opened it. The waypoint the path returns to keeps its place when an earlier waypoint is removed, and the loop moves on to the next when the return waypoint itself is removed. Removing the path's last waypoint still opens the loop.

dkeeton is credited for the ts-patches change the first part follows.
