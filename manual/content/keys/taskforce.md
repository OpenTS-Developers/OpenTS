---
key: TaskForce
summary: The TaskForce whose composition a team of this type recruits and builds toward.
see_also: [Script, House, Group, Max, "system:ai-team-production"]
when_omitted:
  kind: value
  value: none
---

The named TaskForce is the team's roster. Its member lines are the roster slots [recruitment](/systems/ai-team-production/#recruitment) fills one at a time and the unfilled places a computer house builds toward; the total of their quantities is the strength the team counts as full at. A reinforcement group creates exactly those members instead of recruiting them. The TaskForce also supplies the team's recruitment [`Group`](/keys/group/#scope-taskforce) whenever the TeamType leaves its own unset.

```ini title="ai.ini or map file"
[MyRaidTeam] ; example TeamType
Name=Raid team
House=Nod
TaskForce=MyRaidForce ; defined under [TaskForces]
Script=MyRaidScript   ; defined under [ScriptTypes]
```

The value is matched against the registered TaskForce IDs. `<none>` and `none` clear the reference, and a name that no `[TaskForces]` entry registers is not rejected: a fresh, empty TaskForce is created under that name. A team built on an empty one wants nobody, so it never reaches full strength and never starts its script. Outside a campaign the unfilled-team rules dissolve it after a delay; in a campaign it sits there for the rest of the mission.

:::danger[A TeamType with no TaskForce at all is a different matter]
Omitting the key, or clearing it with `<none>`, leaves the reference empty, and the first thing a team's logic does on its first pass is total up the roster without checking that there is one. [AI trigger selection](/systems/ai-team-production/#which-triggers-are-eligible) rejects such a TeamType before it can raise a team, but the [Create Team](/mapping/actions/taction-create-team/) and [Reinforcement (team)](/mapping/actions/taction-reinforcements/) trigger actions do not. A reinforcement never gets that far: the delivery routine totals the roster before the team is built. And an `[AITriggerTypes]` entry naming such a TeamType is worse still: the trigger reads the roster's technology requirement as the scenario loads, before any of this, and faults there.
:::
