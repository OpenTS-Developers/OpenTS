---
key: WaypointAnimationSpeed
summary: Interval between frame changes in the marker drawn on a plotted waypoint.
see_also: [MaxWaypointPathLength]
when_omitted:
  kind: value
  value: "12"
---

```ini title="rules.ini"
[AudioVisual]
WaypointAnimationSpeed=12
```

A plotted waypoint path is the move route a player lays down during play, not the waypoints a mapper places in FinalSun. Each marker on one is drawn from the waypoint cursor's own frames, and a single counter picks the frame for all of them at once. The counter steps every time a timer expires, and the timer is then restarted from this value.

The unit is a system tick of sixteen milliseconds, not a game frame. The default of 12 is therefore a step roughly every fifth of a second, and the markers keep animating at the same speed whatever the game speed is set to. The counter is only advanced once per game frame, so a value small enough to expire within a single frame gives one step per frame and no faster. That ceiling sits around 4 at fifteen frames a second. A value of `0` reaches it immediately.

The setting reaches nothing else. It does not affect how far or how fast anything travels along the path, and [`MaxWaypointPathLength`](/keys/maxwaypointpathlength/) covers how many markers a path may hold.
