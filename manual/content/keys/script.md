---
key: Script
summary: The Script whose team missions a team of this type carries out.
see_also: [TaskForce, House, Tag, "system:ai-team-production"]
when_omitted:
  kind: value
  value: none
---

Each team built from this TeamType is given its own running copy of the named Script and works down its mission list one line at a time; when the list runs out, the team is deleted. Because the copy is per team, several teams of the same type run the same script independently.

```ini title="ai.ini or map file"
[MyRaidTeam] ; example TeamType
Name=Raid team
House=Nod
TaskForce=MyRaidForce ; defined under [TaskForces]
Script=MyRaidScript   ; defined under [ScriptTypes]
```

The value is matched against the registered ScriptType IDs. `<none>` and `none` clear the reference outright, and a name that no `[ScriptTypes]` entry registers is not an error: a fresh ScriptType is registered under that name with no missions in it. A typo therefore produces an empty script rather than a missing one, and a team given an empty script is deleted the first time it tries to advance. A [Reinforcement (team)](/mapping/actions/taction-reinforcements/) is the exception: rather than let an empty script through, it writes an attack-waypoint mission into the ScriptType itself, permanently, for every later team of that type.

:::danger[A TeamType with no Script at all is a different matter]
Omitting the key, or clearing it with `<none>`, leaves nothing behind the running copy. A [Reinforcement (team)](/mapping/actions/taction-reinforcements/) reads the mission list before the group has even been built. A team created any other way crashes the game on the logic pass that flags it into action, where it steps the running copy to the next line. Neither checks first. Point every TeamType at a Script, even an empty one.
:::
