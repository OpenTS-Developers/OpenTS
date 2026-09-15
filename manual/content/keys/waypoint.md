---
key: Waypoint
summary: The waypoint a team of this type starts at and recruits around.
see_also: [House, TaskForce, Recruiter, "system:ai-team-production", "system:ai-team-execution"]
when_omitted:
  kind: value
  value: none
---

The value is a letter label rather than a number. `A` is waypoint `0` and `Z` is `25`; two letters continue from `AA` at `26`, so `AB` is `27` and `BA` is `52`. A value that does not begin with a letter, such as a bare waypoint number, leaves the team with no origin at all.

```ini title="ai.ini or map file"
[MyRaidTeam] ; example TeamType
Name=Raid team
House=Nod
TaskForce=MyRaidForce ; defined under [TaskForces]
Script=MyRaidScript   ; defined under [ScriptTypes]
Waypoint=A
```

The label is turned into a number once, as the section is read, and the cell behind it is looked up fresh every time it is wanted. A label the scenario never placed gives back no cell. Every consumer tests for that, so an unplaced waypoint behaves exactly as no waypoint does.

Five things read the origin, and the first of them does nothing with it. A team of this type takes it as its center point at the moment it is created, and gives it up again on its first logic pass. On that pass [the center is recomputed from the team's members](/systems/ai-team-execution/#the-teams-center) whether or not there are any yet. Nothing measures against the waypoint's cell in between. The other four matter. [Recruitment](/systems/ai-team-production/#recruitment) ranks candidates by their distance from it rather than from the team. A ground member on the Retreat mission heads for the cell calculated on its house's [`Edge`](/keys/edge/) from the origin; without one, that cell is calculated from its own position. An aircraft takes the same mission, but its own routine does nothing with the origin. An air transport carrying a team member picks its landing zone there. A reinforcement group enters the map at it, unless the [Reinforcement (team) at waypoint](/mapping/actions/taction-reinforcements-special/) action names one of its own.

:::danger[Labels past `CW` read beyond the waypoint list]
A scenario holds 101 waypoint slots. `A` through `CT` cover the 98 ordinary ones. `CU`, `CV` and `CW` are the three the engine reserves: the scenario's home cell, its reinforcement cell and its special airdrop cell. `CX` and anything after it convert to a number past the end of that list, and the origin cell is then read from whatever lies beyond it.
:::
