---
title: AI triggers and team production
summary: "Draws one weighted AI trigger per house on a timer and turns the TeamTypes it names into recruitment and production demand."
category: ai-teams
keys:
  - AIHateDelays
  - AITriggerFailureWeightDelta
  - AITriggerSuccessWeightDelta
  - AITriggerTrackRecordCoefficient
  - AreTeamMembersRecruitable
  - Autocreate
  - AutocreateTime
  - CompEasyBonus
  - DissolveUnfilledTeamDelay
  - FillEarliestTeamProbability
  - Group
  - House
  - IgnoreGlobalAITriggers
  - IsBaseDefense
  - LooseRecruit
  - Max
  - MaximumAIDefensiveTeams
  - MinimumAIDefensiveTeams
  - Prebuild
  - Priority
  - RatioAITriggerTeam
  - Recruitable
  - Recruiter
  - Reinforce
  - Script
  - TaskForce
  - TeamDelays
  - TechLevel
  - TotalAITeamCap
  - UseMinDefenseRule
  - Waypoint
related:
  - type: system
    id: ai-base-building
  - type: format
    id: ai_triggers
  - type: format
    id: teamtypes
  - type: format
    id: taskforces
  - type: format
    id: scripts
  - type: action
    id: TACTION_BEGIN_AI_TRIGGERS
  - type: action
    id: TACTION_STOP_AI_TRIGGERS
  - type: action
    id: TACTION_SET_AI_TRIGGER_TEAM_RATIO
  - type: mission
    id: TMISSION_SUCCESS
---

## TeamTypes and AI triggers in brief

A **team** is a group of infantry, vehicles and aircraft that one house owns and that acts together. A computer house's raids, attacks and base defense all arrive as teams.

A scenario does not hold teams. It holds **TeamTypes**, the definitions that teams are created from. A new team fills up with objects its house already owns or builds for the purpose. When the team reaches full strength it is **started**, or set under way. It pauses to gather, as [team execution](/systems/ai-team-execution/#the-reform-delay) describes, and then works through its Script. What the team does after that is outside this page.

A TeamType is one INI section that points at two others. The table shows what each of the three sections contributes to the mechanics on this page.

| Where it is written | What it contributes |
| --- | --- |
| The [TaskForce](/mapping/task-forces/) section named by [`TaskForce=`](/keys/taskforce/) | The roster: up to six `<count>,<ObjectType ID>` lines. The sum of the counts is the team's **full strength**, the number of members it wants. Full strength counts members, not hit points. |
| The [Script](/mapping/scripts/) section named by [`Script=`](/keys/script/) | The team missions the team carries out in order once it starts. Reaching the [Success](/mapping/missions/tmission-success/) mission is the only thing [the track record](#the-track-record) counts as a win. |
| The [TeamType](/mapping/team-types/) section itself | Which TaskForce and Script to use, the country [`House=`](/keys/house/) gives the team to, the [`Waypoint=`](/keys/waypoint/) where the team starts and recruits, and the settings in the next table. |

A team raised by an AI trigger belongs to the house whose pass raised it, whatever country `House=` names. `House=` decides the owner only when a trigger action creates the team.

Eight more TeamType settings affect the decisions on this page. The table links each one to the section that covers it.

| Setting | Where it acts |
| --- | --- |
| [`Max=`](/keys/max/) | How many teams of the type one house may hold. A TeamType that never sets it is [rejected before the draw](#which-triggers-are-eligible), so no AI trigger can raise it. |
| [`IsBaseDefense=`](/keys/isbasedefense/#scope-teamtype) | Marks the type as defensive. [The team budget](#the-team-budget) counts, caps and deletes defensive teams. |
| [`Reinforce=`](/keys/reinforce/) | Keeps [recruitment](#recruitment) open after the team has started. |
| [`Recruiter=`](/keys/recruiter/) | Lets [recruitment](#recruitment) take objects from outside the team's group. |
| [`Group=`](/keys/group/#scope-teamtype) | The group [recruitment](#recruitment) draws from, and the group each member is given when it joins. |
| [`Priority=`](/keys/priority/#scope-teamtype) | Whether [recruitment](#recruitment) may take a member from another team: only from a team of strictly lower priority. |
| [`Autocreate=`](/keys/autocreate/) | Sets the autocreate mark [from the start](#from-suggestion-to-team). |
| [`AreTeamMembersRecruitable=`](/keys/areteammembersrecruitable/) | The value a joining member's autocreate recruitable state is set to. |

**Autocreated** is a mark on a TeamType. It does not make the engine create teams by itself. `Autocreate=yes` sets the mark when the TeamType is read, and an AI trigger drawing the TeamType sets it permanently. The mark only decides which of an object's two recruitable states a team of that type checks; [recruitment](#recruitment) covers the pair.

An **AI trigger** is one line in [`[AITriggerTypes]`](/mapping/ai-triggers/). It is unrelated to the tags, events and actions a map uses: nothing springs it, and it does nothing except name teams. Each line names a first TeamType and optionally a second, one condition, three weights, and a set of gate fields: difficulty flags, a side, a campaign owner and a skirmish flag. A house never picks a team directly. It draws a trigger and creates the TeamTypes that trigger names.

The example below shows one trigger and the three sections behind it.

```ini title="AI.INI, AIFS.INI, or map file"
[TaskForces]
0=MyRaidForce

[MyRaidForce] ; example TaskForce
Name=Raid force
0=3,E1
1=1,TTNK

[ScriptTypes]
0=MyRaidScript

[MyRaidScript] ; example Script
Name=Raid
0=0,1  ; attack anything
1=49,0 ; Success; without it every raid team records a failure

[TeamTypes]
0=MyRaidTeam

[MyRaidTeam] ; example TeamType
Name=Raid team
House=Nod
TaskForce=MyRaidForce
Script=MyRaidScript
Max=4
Priority=8

[AITriggerTypes]
MyRaidTrigger=Raid a blacked-out base,MyRaidTeam,<all>,0,3,GAPOWR,0000000000000000,8,1,20,1,0,0,0,<none>,0,1,1
```

The [AI triggers](/mapping/ai-triggers/) page covers the fields of the trigger record. In this example:

- `Max=4` is required. Without `Max`, no AI trigger can raise the TeamType.
- `Priority=8` lets a raid team take a member from a team left at the default priority of `7`.
- Condition type `3` holds while the enemy's power output is below its drain. This condition ignores the comparison block and the condition object, so `GAPOWR` is never counted.
- The last three fields leave the trigger off at Easy and on at Medium and Hard.

## The pass in outline

Each house raises teams on its own countdown. When the countdown reaches zero, the house draws one AI trigger at random from those whose gates and condition currently hold. Each trigger's chance depends on a weight that tracks how its past teams fared. The house then creates one team from the trigger's first TeamType, and one from its second TeamType if it names one.

The new teams fill themselves from objects the house owns. A computer house [orders the members still missing](#production-demand) through its ordinary production. A human house orders no production for its teams.

## When the pass runs

The countdown first runs for `175 * <the house's position in the house list> + TeamDelays` [frames](/glossary/#frame), so houses do not all run their pass on the same frame. After every pass it restarts at [`TeamDelays`](/keys/teamdelays/) alone.

Every house runs the pass, computer or human. The pass reaches the AI triggers only while both of these hold:

- a roll from `1` to `100` comes out at or below the house's [`RatioAITriggerTeam`](/keys/ratioaitriggerteam/);
- the house's AI-trigger switch is on.

If either fails, the pass raises no team and the countdown restarts. A failed roll does not raise some other kind of team instead.

The switch starts off for every house. Four events change it:

| Event | Effect |
| --- | --- |
| [AI triggers begin...](/mapping/actions/taction-begin-ai-triggers/) | On, for the house the action names |
| [AI triggers stop...](/mapping/actions/taction-stop-ai-triggers/) | Off, for the house the action names |
| An MCV deploying into a construction yard | On, for a non-human house outside a campaign game |
| A house passing from a human player to the computer | On, if the house still holds a construction yard |

:::caution[Outside a campaign the switch waits for an MCV]
Outside a campaign game, deploying an MCV into a construction yard is the only way a computer house switches itself on. A computer house that starts a skirmish or multiplayer game with its base already built never draws an AI trigger, unless a map trigger action turns the switch on.
:::

The begin action works on a human house as well. That house then raises teams from AI triggers like a computer house, but orders no production to fill them.

## The team budget

Before examining any trigger, the house counts the teams it owns and how many of them come from [`IsBaseDefense=yes`](/keys/isbasedefense/#scope-teamtype) TeamTypes. Every team the house owns is counted, however it was created.

The two counts select one of two cases. Only the second case deletes a team.

| The house's counts | What this pass does |
| --- | --- |
| Fewer teams than [`TotalAITeamCap`](/keys/totalaiteamcap/), **or** fewer defensive teams than half the team count, rounded down | Nothing is deleted. Defensive triggers are skipped in this pass if the defensive count is above [`MaximumAIDefensiveTeams`](/keys/maximumaidefensiveteams/). |
| At least `TotalAITeamCap` teams, **and** at least half of them, rounded down, defensive | The house deletes its oldest defensive team, which lowers its team count by one. Defensive triggers are skipped in this pass whatever the defensive count. A house holding no defensive team deletes nothing and skips nothing. |

The house then goes on to draw a trigger only if its team count is below `TotalAITeamCap`.

:::caution[Deleting the oldest defensive team records an outcome]
The deleted team records a success or a failure for every trigger that names its TeamType first, as any removed team does. This happens before the house draws, so the draw in the same pass already uses the changed weights.
:::

## Which triggers are eligible

The house tests each trigger against these gates in order. The first gate that fails rejects the trigger.

1. It names a first TeamType.
2. It is defensive, or the house is not [restricted to defensive triggers](#defensive-teams-and-the-enemy).
3. It is not defensive, or [the team budget](#the-team-budget) is not skipping defensive triggers in this pass.
4. It is a map-local trigger, or the scenario does not set [`IgnoreGlobalAITriggers=yes`](/keys/ignoreglobalaitriggers/).
5. It is enabled.
6. Outside a campaign game, it is marked as available in skirmish.
7. Its flag for the current [difficulty](#difficulty) is set.
8. In a campaign game, its owner is not `<none>`, and is either `<all>` or this house's country.
9. Its side field is `0` or less, or it gives the position in `[Sides]`, counting from `1`, of the side of the country the house [acts as](/keys/actslike/). A value that names no side rejects the trigger.
10. The house's tech level is at least the trigger's tech level requirement.
11. Its [condition](#conditions) holds.
12. The house can build every member of both TeamTypes' TaskForces.
13. Neither TeamType has reached its [`Max`](/keys/max/) among the teams this house owns.

The tech level requirement is not a field of the record. It is the highest `TechLevel` among the members of the trigger's TaskForces, so a trigger requires what its teams require. A member with `TechLevel=-1` raises the requirement to `11`, which rejects the trigger for every house below tech level 11.

:::danger[Give every TeamType a TaskForce]
Set [`TaskForce=`](/keys/taskforce/) on every TeamType that an AI trigger names. If one has no `TaskForce=`, the game crashes while it reads that trigger during scenario loading.
:::

:::caution[Outside a campaign the owner is not read]
The owner test runs only in a campaign game. In skirmish and multiplayer games, every house with its switch on considers every enabled trigger, whatever owner the record names. The side field, the condition and gate 12 are then the only gates that keep a trigger to one kind of house. Gate 12 does so because a house cannot build a member type its country cannot own.
:::

:::caution[Outside a campaign every team member must be buildable]
In a campaign game, a member type the house cannot build still passes if the house owns an object of that type that the team could recruit. Outside a campaign there is no such substitute. One member type the house cannot currently build rejects the trigger, whether it lacks a working factory, a prerequisite, the tech level, or the right to own the type.
:::

## Difficulty

The three difficulty flags on a trigger mean Easy, Medium and Hard as the player chose them. This holds for every house in a campaign game, and for computer houses outside one.

In a campaign game, the house tests the flag that matches the scenario's difficulty. Outside a campaign, the house tests the flag opposite to its [difficulty slot](/systems/difficulty/#from-the-setting-to-a-slot). A computer house's slot is already the inverse of the setting the player chose, so the two inversions cancel. The table traces one computer house.

| Setting the player chose | Slot the computer house holds | Flag the trigger must set |
| --- | --- | --- |
| Easy | 2, the `[Difficult]` section | Easy |
| Normal | 1, the `[Normal]` section | Medium |
| Hard | 0, the `[Easy]` section | Hard |

A human house outside a campaign always holds slot 1, so it always tests the Medium flag.

A computer seat's `[HouseHandicaps]` value in a [launch file](/formats/spawn-ini/#who-is-playing) sets its slot directly. It replaces both the inversion and the bonus described below. The flag the house tests then follows the slot column of the table, so a value of `0` tests the Hard flag.

The per-difficulty lists in `[General]` are not inverted a second time. Each list is indexed by the house's slot, so for a computer house entry 0 applies at the hardest setting and entry 2 at the easiest. The lists are [`TeamDelays`](/keys/teamdelays/), [`TotalAITeamCap`](/keys/totalaiteamcap/), [`MinimumAIDefensiveTeams`](/keys/minimumaidefensiveteams/), [`MaximumAIDefensiveTeams`](/keys/maximumaidefensiveteams/), [`FillEarliestTeamProbability`](/keys/fillearliestteamprobability/) and [`AIHateDelays`](/keys/aihatedelays/).

:::danger[Give each list three entries]
None of those six keys has a built-in value. If no rules file sets one, the list is empty, and looking up a house's entry reads memory outside the list. A list with fewer than three entries does the same for the missing slots. The result is an unpredictable value or a crash.
:::

:::caution[More than one human player shifts the slot]
Outside a campaign, [`CompEasyBonus=yes`](/keys/compeasybonus/) drops a computer house one slot when the game has more than one human player, unless the house is already in slot 0. The list entry and the tested difficulty flag both move with the slot. At Normal with two human players, a computer house therefore reads entry 0 of each list and tests each trigger's Hard flag.
:::

## Defensive teams and the enemy

A trigger is defensive when its first TeamType is [`IsBaseDefense=yes`](/keys/isbasedefense/#scope-teamtype) and its second TeamType is either absent or defensive as well.

A house considers only defensive triggers while either of these holds:

- the house has no enemy;
- **All of:** [`UseMinDefenseRule=yes`](/keys/usemindefenserule/), and the house has fewer defensive teams than [`MinimumAIDefensiveTeams`](/keys/minimumaidefensiveteams/).

:::caution[The trigger's base defense field decides nothing]
The base defense field in the trigger record is stored but never read. Only the `IsBaseDefense` settings of the TeamTypes a trigger names make it defensive.
:::

A house's enemy is the house it is angriest with, and [damage is what raises that anger](/systems/base-attacked/#anger-and-the-declared-enemy). Outside a campaign, a computer house without an enemy also picks one on a timer. Nothing picks one on a timer in a campaign. A campaign house has no enemy until damage raises its anger or a map trigger action makes it an enemy of another house. Until then it considers only defensive triggers.

## Conditions

The [trigger record](/mapping/ai-triggers/) page lists the condition types. At runtime they measure the following:

- **The two owning conditions** count the objects of the condition object's type that the enemy, or this house, currently owns. The count is compared against the record's number with the record's comparator. If the condition object ID is not recognized, the count is `0`.
- **The yellow power condition** ignores the record's number and comparator. It holds while the enemy's power output minus its drain is below `100`.
- **The red power condition** holds while the enemy's power output minus its drain is below `0`.
- **The money condition** compares the enemy's available credits against the record's number with the record's comparator.

Every condition except the one that counts this house's objects reads the enemy. A house without an enemy can therefore pass only a defensive trigger that has no condition or counts the house's objects.

## The weighted draw

The house draws one trigger from those that passed every gate. Each trigger's chance is in proportion to its current weight. Weights count only as whole numbers: the three weights in a record are truncated as it is read, and the current weight is truncated again for the draw. A trigger whose current weight is below `1` passes its gates but can never be drawn.

## The track record

Each trigger has a current weight, a minimum weight and a maximum weight, and two counters: the runs it has had and the runs that succeeded. The counters start at `0`. Each weight is `1` unless the record supplies it.

When a team is removed for any reason, every trigger whose **first** TeamType is that team's type records an outcome. The outcome is a success if the team reached the [Success](/mapping/missions/tmission-success/) team mission at any point in its Script, and a failure otherwise.

Each outcome adds a history term and a fixed delta to the current weight. The history term is the number of successes so far minus half the number of runs so far, counted before this outcome. It is `0` at a success rate of exactly one in two, and it grows with the number of runs, so a long record moves the weight further than a short one. [`AITriggerTrackRecordCoefficient`](/keys/aitriggertrackrecordcoefficient/) scales the history term for failures only.

| Outcome | History term added | Delta added | Counters afterward |
| --- | --- | --- | --- |
| Success | Successes minus half the runs, or `0` if that is negative | [`AITriggerSuccessWeightDelta`](/keys/aitriggersuccessweightdelta/) | One more run, one more success |
| Failure | Successes minus half the runs, multiplied by `AITriggerTrackRecordCoefficient`, or `0` if that product is positive | [`AITriggerFailureWeightDelta`](/keys/aitriggerfailureweightdelta/) | One more run |

After each outcome, the weight is clamped between the trigger's minimum and maximum. A starting weight outside that range stays as written until the trigger's first outcome.

:::caution[A team that never runs Success records a failure]
Only the Success team mission marks a team as successful. A team that fights well but is destroyed before reaching that mission records a failure. So does a team that dissolves because it never filled, and the defensive team [the team budget](#the-team-budget) deletes.
:::

Outcomes are keyed on the team's TeamType alone. The engine does not check which trigger raised the team or which house owns it. As a result:

- every trigger that names the TeamType first records the same outcome;
- one house's outcomes change the weights every other house draws with;
- a team that a map trigger action created also records outcomes;
- a team created from a trigger's second TeamType records outcomes only for triggers that name that TeamType first.

## From suggestion to team

Before creating anything, the house checks the teams it already owns. If one of them is reforming or not under way, and its TeamType is either of the drawn trigger's TeamTypes, the house discards the whole draw. Neither TeamType is created.

Each TeamType that survives is marked as autocreated for the rest of the scenario. [`Autocreate=yes`](/keys/autocreate/) sets the same mark from the start. [Recruitment](#recruitment) reads the mark. So does the campaign substitute in gate 12, because it asks whether the team could recruit the object it finds.

The house then creates one team from each surviving TeamType. Each creation checks `Max` again, and this check differs from the eligibility gate:

- In a campaign game, it counts the teams of that type that any house owns.
- Outside a campaign, it counts the teams of that type this house owns, the same count the eligibility gate used.

A campaign house can therefore pass the eligibility gate and still create nothing, because other houses' teams of the same type count toward `Max`.

## Recruitment

A team fills its TaskForce by taking objects its house already owns. It looks for them on each of its [logic turns](/systems/ai-team-execution/#the-logic-turn) while all of this holds:

- **Any of:**
  - the team is not under way: it has not started yet, or it was [sent back to regroup](/systems/ai-team-execution/#every-regroup-rewinds-the-script), which happens only to a `Reinforce=yes` team;
  - **All of:** it is short of full strength, and its TeamType is [`Reinforce=yes`](/keys/reinforce/).
- **Any of:**
  - the team belongs to a computer house;
  - the team has never reached full strength and has never started.

The second group closes recruitment on a human house's team for good once the team reaches full strength or starts. Losing members afterward does not reopen it, and neither does `Reinforce=yes`.

On each turn, the team adds at most one member for each roster line that is still short. Passengers inside a recruited transport do not join the team with it.

The team takes the nearest candidate that passes every check below. Distance is measured from the cell of the TeamType's [`Waypoint`](/keys/waypoint/), or from the team's center when it has no waypoint. An object is considered at all only under one of these:

- the team's group is `-2`;
- the object is in the team's group;
- the TeamType is [`Recruiter=yes`](/keys/recruiter/).

A candidate from outside the team's group counts as 50 cells farther away than it is. The team therefore takes it only if it is more than 50 cells nearer than every candidate from the group.

The team's group is the TeamType's [`Group`](/keys/group/#scope-teamtype). While the TeamType leaves `Group` at `-1`, the TaskForce's [`Group`](/keys/group/#scope-taskforce) is used instead. Each member's group is set to the team's group when it joins.

A considered candidate must pass these checks, in this order:

1. It is not already on this team.
2. It is alive and on the map, it is not in radio contact, and its house is the team's house.
3. It has no mission, or its current mission is one that [allows recruitment](/keys/recruitable/).
4. Its recruitable state for this kind of team is set, as described below.
5. If it is on another team, that team's [`Priority`](/keys/priority/#scope-teamtype) is strictly lower than this team's.
6. It is not an armed aircraft that is out of ammunition.
7. Its type matches one of the TaskForce's roster lines, and that line still has room.

Every object has two recruitable states: one for ordinary teams and one for autocreated teams. A team whose TeamType is not marked autocreated checks only the ordinary state. A marked team checks only the autocreate state. Both states start set, unless a map's [placed-object record](/formats/scenario-objects/) clears one.

Joining any team sets the object's autocreate state to that TeamType's [`AreTeamMembersRecruitable`](/keys/areteammembersrecruitable/). A team with `AreTeamMembersRecruitable=no` keeps its members out of autocreated teams, even after the team itself is gone. The lock lasts until the object joins a team whose TeamType sets `AreTeamMembersRecruitable=yes`.

A team with no members is deleted in either of these cases:

- it has reached full strength or started at some point;
- outside a campaign, more than [`DissolveUnfilledTeamDelay`](/keys/dissolveunfilledteamdelay/) frames have passed since it was created.

The second case is the usual end of a team whose TaskForce the house could not fill. In a campaign, an empty team that never filled is not deleted this way. It stays and keeps recruiting. Until it fills, it counts toward `Max` and the team budget, and it makes the house [discard every draw](#from-suggestion-to-team) that names its TeamType.

## Production demand

Only computer houses order production for their teams. [The base planner](/systems/ai-base-building/) chooses structures. Vehicles, infantry and aircraft each have a separate chooser, and all three work the same way.

Two cases come before the steps below. The vehicle chooser may order a harvester instead when the house is short of harvesters for its refineries. A chooser that already has a type selected and not yet built does nothing.

A computer house normally runs the base planner and all three choosers together. When its credits fall below [`AIAlternateProductionCreditCutoff`](/keys/aialternateproductioncreditcutoff/), it alternates between structures and units instead. While structures have their turn, the three choosers run only if the base planner has chosen no structure, or one the house cannot build. In a campaign game no house alternates.

1. **Demand.** A team of this house adds to demand in either of these cases:
   - **All of:** its TeamType is `Reinforce=yes`, and it is short of full strength;
   - **All of:** it is not forced active, and it has never reached full strength or started. Only a reinforcement group is forced active.

   Each unfilled roster place of a type the chooser builds adds one to that type's demand. For each type, the house also notes the creation time of the oldest team that wants it.

2. **Supply.** Each object of the house that is free to recruit takes one off its type's demand. An object is free when it is on the map, on no team, on a mission that allows recruitment, and its ordinary recruitable state is set.
3. **Candidates.** A type is a candidate while its demand is above zero, the house can build it, and its cost is within the house's available credits.
4. **Choice.** With a chance of [`FillEarliestTeamProbability`](/keys/fillearliestteamprobability/) percent, the house builds the candidate wanted by the oldest team. Otherwise it picks from the candidate list at random, with equal chances.

Supply does not check the autocreate recruitable state. An object that `AreTeamMembersRecruitable=no` locked away still takes one off demand, although no autocreated team can recruit it.

The random pick does not favor the types in greatest demand. The candidate list is emptied only when the house reaches a candidate with more demand than every candidate before it. The list therefore holds that type and every candidate registered after it, including candidates with less demand. Which types those are depends on the order the types are registered in.

Demand is counted for every registered InfantryType, UnitType and AircraftType, however many a mod defines.

## Parsed settings without effect

A TeamType's [`Prebuild`](/keys/prebuild/) and [`LooseRecruit`](/keys/looserecruit/) are read but have no effect, and neither does [the TeamType's `TechLevel`](/keys/techlevel/#scope-teamtype). A trigger's tech level requirement comes from its TaskForce members instead. [`AutocreateTime`](/keys/autocreatetime/) in `[AI]` is read but reaches only disabled code.
