---
key: House
summary: The country whose house in the match owns teams built from this TeamType.
see_also: [TaskForce, Script, Waypoint, "system:ai-team-production"]
when_omitted:
  kind: value
  value: none
---

The value names a country, matched against every registered country's ID and its [`Name=`](/keys/name/) string, and the team is given whichever house in the match is playing it. It may instead name a [spawn house](/formats/scenario-objects/#spawn-houses), `Spawn1` through `Spawn8` or `<Player @ A>` through `<Player @ H>`, and the team is given the house that starts at that position. A country no house is playing resolves to nothing, and so do `<none>` and a spawn house nobody holds. An unrecognized name is not rejected: it registers a fresh country under that name, which by definition nobody is playing, while a spawn house registers none. Omitting the key keeps whatever an earlier layer set.

```ini title="ai.ini or map file"
[MyRaidTeam] ; example TeamType
Name=Raid team
House=Nod
TaskForce=MyRaidForce ; defined under [TaskForces]
Script=MyRaidScript   ; defined under [ScriptTypes]
```

The owner settles who the team's members are created for and who they may be recruited from; a candidate of any other house is refused outright. It is also the house the team counts against for the defensive-team census and the per-house team limit.

:::caution[An AI trigger ignores this setting]
A team raised through the [AI trigger pass](/systems/ai-team-production/#from-suggestion-to-team) is created for the house whose pass raised it, whatever country this key names. The setting decides the owner only where a team is created from the TeamType alone: the [Reinforcement (team)](/mapping/actions/taction-reinforcements/) and [Create Team](/mapping/actions/taction-create-team/) trigger actions. A [Change team](/mapping/missions/tmission-teamchange/) mission hands the new team the old team's house instead.
:::

:::caution[A team with no owner is never raised]
Both paths that read this key refuse a TeamType whose owner resolved to nothing: the reinforcement delivers no group and the Create Team action creates no team. A misspelled country name, a country the rules register but no house in this match is playing, or a spawn house nobody holds is enough to reach this.
:::
