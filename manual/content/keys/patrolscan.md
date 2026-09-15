---
key: PatrolScan
summary: How often a patrolling team looks around for something to attack.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: ".016"
---

The value is given in minutes and turned into a whole number of frames, so the default is 14 frames at 900 frames to the game minute. One thing reads it: the [Patrol to waypoint...](/mapping/missions/tmission-patrol/) team mission. On each qualifying frame it asks the team's leader for the greatest threat within the leader's own [`GuardRange`](/keys/guardrange/), and falls back on the leader's weapon range only where `GuardRange` is zero. The leader is the first member that has reached the team, not one still traveling to its regroup point. A threat found turns the patrol into an attack. Finding nothing puts the team back on course for its waypoint.

The interval is measured against the game frame rather than against each team's own clock, so every patrolling team in the scenario scans on the same frames.

:::danger[An interval shorter than one frame divides by zero]
The frame counter is divided by the interval without any check that the interval is at least one. The interval is the value multiplied out and truncated, so `PatrolScan=0`, or any value smaller in magnitude than `1/900` (about `0.0011`), leaves it at zero. The first team to reach a Patrol mission then divides by zero.
:::
