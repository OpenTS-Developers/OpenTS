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

This section introduces the two entities the rest of the page turns on. Anyone already writing TeamTypes and `[AITriggerTypes]` records can skip to [the pass in outline](#the-pass-in-outline).

A **team** is a group of infantry, vehicles and aircraft that one house owns and that acts together; a computer house's raids, its attacks and its base defense all arrive as teams. Teams are not written into a scenario directly. What a scenario carries is a **TeamType** — the definition a team is built from — and a team is created from that definition and then filled, either with objects the house already owns or with objects it builds for the purpose. A team that reaches full strength is **started**, or set in motion, and begins working down its Script — after a pause to close ranks, which is the first thing [team execution](/systems/ai-team-execution/#the-reform-delay) covers. What it does from that point onward is outside this page.

A TeamType is one INI section that points at two others. The table names the three sections and what each one contributes to the mechanics below, so a reader tracing a misbehaving team knows which of the three to open.

| Where it is written | What it contributes |
| --- | --- |
| The [TaskForce](/mapping/task-forces/) section named by [`TaskForce=`](/keys/taskforce/) | The roster: up to six `<count>,<ObjectType ID>` lines. The total of those counts is the team's **full strength** — the number of members it wants. It is a roster figure and has nothing to do with health |
| The [Script](/mapping/scripts/) section named by [`Script=`](/keys/script/) | The ordered team missions the team carries out once it starts. One of them matters here: reaching [Success](/mapping/missions/tmission-success/) is the only thing [the track record](#the-track-record) counts as a win |
| The [TeamType](/mapping/team-types/) section itself | Which of those two to use, which country [`House=`](/keys/house/) hands the team to, which [`Waypoint=`](/keys/waypoint/) it starts at and recruits around, and the settings in the next table |

A team raised through an AI trigger belongs to the house whose pass raised it, whatever country `House=` names; that setting decides the owner only where a trigger action creates the team instead.

Eight further TeamType settings reach the decisions on this page. The table points each one at the section that covers it, so a reader who came for one setting can go straight there.

| Setting | Where it acts |
| --- | --- |
| [`Max=`](/keys/max/) | How many teams of the type one house may hold. A TeamType that never sets it is [rejected before the draw](#which-triggers-are-eligible), so no AI trigger can raise it |
| [`IsBaseDefense=`](/keys/isbasedefense/#scope-teamtype) | Marks the type defensive, which is what [the team budget](#the-team-budget) counts, caps and culls |
| [`Reinforce=`](/keys/reinforce/) | Keeps [recruitment](#recruitment) open after the team has started |
| [`Recruiter=`](/keys/recruiter/) | Lets [recruitment](#recruitment) look outside the team's group |
| [`Group=`](/keys/group/#scope-teamtype) | The group [recruitment](#recruitment) matches on, and the group it stamps onto each member |
| [`Priority=`](/keys/priority/#scope-teamtype) | Which of two teams wins a member they both want |
| [`Autocreate=`](/keys/autocreate/) | Sets the autocreate mark [from the start](#from-suggestion-to-team) |
| [`AreTeamMembersRecruitable=`](/keys/areteammembersrecruitable/) | What a joining member's autocreate-recruitable state is overwritten with |

**Autocreated** is a mark on the TeamType rather than a description of anything the engine does on its own. `Autocreate=yes` sets it as the rules are read and being drawn by an AI trigger sets it too, permanently. All the mark decides is which of an object's two recruitable states a team of that type consults; [recruitment](#recruitment) covers the pair.

An **AI trigger** is one line in [`[AITriggerTypes]`](/mapping/ai-triggers/). It has nothing to do with the tags, events and actions a map uses: nothing springs it, and it does nothing of its own beyond naming teams. Each line carries a first TeamType and optionally a second, one condition to test, three weights, and a set of gate fields — difficulty flags, a side, a campaign owner, a skirmish flag. A house never asks for a team directly. It asks for a trigger, and takes whichever TeamTypes that trigger names.

Written out in full, one trigger and the three sections behind it look like this.

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

The trigger record's fields are covered by the [AI triggers](/mapping/ai-triggers/) page. Three of the values above are worth naming here: `Max=4` is what makes this TeamType reachable at all, `Priority=8` lets a raid team take a member off a team left at the default `7`, and condition type `3` fires while the enemy's power output sits below its drain, so the comparison block is unused here and the condition object is never counted.

## The pass in outline

A house raises teams on a countdown of its own. Each time that countdown reaches zero the house asks for one suggestion: a single AI trigger is drawn at random from those whose gates and condition currently hold, weighted by a record the trigger keeps of how its past teams fared, and one team is created from each TeamType it names — the first, and the second when it has one.

Filling those teams is then the teams' own work, and the members they are still missing are ordered through the same production the house runs for everything else. A computer house orders structures, vehicles, infantry and aircraft together in one pass; a house whose credits have fallen below [`AIAlternateProductionCreditCutoff`](/keys/aialternateproductioncreditcutoff/) narrows to one class at a time, and no house narrows in a campaign game.

## When the pass runs

The countdown starts at `175 * <the house's position in the house list> + TeamDelays`, so houses do not all run their pass on the same frame, and it is reset to [`TeamDelays`](/keys/teamdelays/) alone after every pass. Every house runs the pass, computer or human. Whether it reaches the AI triggers depends on two further gates: a `1` to `100` roll must come out at or below the house's [`RatioAITriggerTeam`](/keys/ratioaitriggerteam/), and the house's AI-trigger switch must be on. When either fails the pass ends with no suggestion and the countdown restarts anyway; a failed roll does not raise some other kind of team instead.

The switch starts off for every house, and exactly four things turn it on or off. The table shows what each of the four does to it; outside a campaign game the third row is the only one that fires on its own, which is what the caution below turns on.

| Written by | Effect |
| --- | --- |
| [AI triggers begin...](/mapping/actions/taction-begin-ai-triggers/) | On, for the house the action names |
| [AI triggers stop...](/mapping/actions/taction-stop-ai-triggers/) | Off, for the house the action names |
| An MCV deploying into a construction yard | On, for a non-human house outside a campaign game |
| A house passing from a human player to the computer | On, provided the house still holds a construction yard |

:::caution[Outside a campaign the switch waits for an MCV]
The deploy path is the only one of the four that fires by itself outside a campaign game, so a computer house that starts a skirmish or multiplayer session with its base already built — rather than with an MCV to deploy — never springs an AI trigger. Since the pass is not restricted to computer houses, the trigger action switches a human house into it just as readily.
:::

## The team budget

Before any trigger is examined the house counts its live teams and how many of them come from an [`IsBaseDefense=yes`](/keys/isbasedefense/#scope-teamtype) TeamType. Every team the house owns is counted, including teams the scenario placed.

Those two counts then select one of two branches. The table sets both out; the difference that matters is that only the second branch ever deletes a team, and that the second branch passes defensive triggers over regardless of how few the house holds.

| The house's counts | What this pass does |
| --- | --- |
| Team count below [`TotalAITeamCap`](/keys/totalaiteamcap/), **or** defensive teams below half the team count rounded down | Nothing is deleted. Defensive triggers are passed over for this pass only while the defensive count is above [`MaximumAIDefensiveTeams`](/keys/maximumaidefensiveteams/) |
| Neither: the house is at or above `TotalAITeamCap` **and** at least half its teams are defensive | The house's oldest defensive team is deleted outright, the team count drops by one, and defensive triggers are passed over for this pass whatever the defensive count is. A house holding no defensive team at all deletes nothing and passes nothing over |

Selection then proceeds only while the team count is below `TotalAITeamCap`.

:::caution[Culling the oldest defensive team records an outcome]
That team is deleted through the ordinary team teardown, so it feeds a success or a failure into the track record of every trigger that names its TeamType first — before this pass has even assembled its candidates.
:::

## Which triggers are eligible

Each trigger is put through these gates in order, and the first one that fails rejects it:

1. It names a first TeamType.
2. It is defensive, or the house is not [restricted to defensive triggers](#defensive-teams-and-the-enemy).
3. It is not defensive, or defensive triggers were not passed over above.
4. It is a map-local trigger, or the scenario does not set [`IgnoreGlobalAITriggers=yes`](/keys/ignoreglobalaitriggers/).
5. It is enabled.
6. Outside a campaign game, it is marked as available in skirmish.
7. Its flag for the current difficulty is set.
8. In a campaign game, its owner is not `<none>` and is either `<all>` or this house's country.
9. Its side field is unrestricted, or it names, counted from one, the position in `[Sides]` of the side of the country the house [acts as](/keys/actslike/); a value naming no side never fires.
10. Its tech level requirement is within the house's tech level.
11. Its condition holds.
12. Every member of both TeamTypes' TaskForces is something this house can build.
13. Neither TeamType has reached its [`Max`](/keys/max/).

The tech level requirement is not taken from the record: it is the highest `TechLevel` among the members of the trigger's TaskForces, so a trigger demands exactly what its teams demand — with one trap: a member whose `TechLevel` is `-1` pushes the requirement to `11`, silently making the trigger unspringable.

:::caution[Outside a campaign the owner is not consulted]
The owner test runs only in a campaign game. In a skirmish or multiplayer session every house with the switch on evaluates every enabled trigger, whatever name the record carries, and the side field and the condition are the only restrictions left that can tie a trigger to one kind of house.
:::

:::caution[Outside a campaign every team member must be buildable]
The buildable test accepts a recruitable object the house already owns in place of one it can build, but only in a campaign game. Outside a campaign one member the house cannot currently build — a type held back by prerequisites, tech level or ownership — rejects the trigger outright. A TeamType with no TaskForce at all is rejected everywhere.
:::

## Difficulty

The three difficulty flags on a trigger mean Easy, Medium and Hard as the player chose them, and that reading holds for every house in a campaign game and for computer houses outside one.

In a campaign game the flag tested is simply the one matching the scenario's difficulty. Outside a campaign it takes two steps. The test reads the house's own [difficulty slot](/systems/difficulty/#from-the-setting-to-a-slot), which for a computer house is already the inverse of the setting the player chose, and then asks for the flag at the opposite end again. The table traces one computer house through both steps. Its left and right columns agree, which is the point: the two inversions cancel, and the flags mean what they are named.

| Setting the player chose | Slot the computer house holds | Flag the trigger must set |
| --- | --- | --- |
| Easy | 2, the `[Difficult]` section | Easy |
| Normal | 1, the `[Normal]` section | Medium |
| Hard | 0, the `[Easy]` section | Hard |

A human house switched into the pass outside a campaign sits in slot 1 whatever the session was set to, so it always tests the Medium flag.

The per-difficulty lists in `[General]` get no second inversion. Each is indexed with the raw slot, so entry 0 is the hardest game setting and entry 2 the easiest: [`TeamDelays`](/keys/teamdelays/), [`TotalAITeamCap`](/keys/totalaiteamcap/), [`MinimumAIDefensiveTeams`](/keys/minimumaidefensiveteams/), [`MaximumAIDefensiveTeams`](/keys/maximumaidefensiveteams/), [`FillEarliestTeamProbability`](/keys/fillearliestteamprobability/) and [`AIHateDelays`](/keys/aihatedelays/).

:::danger[An omitted list is not a fallback]
None of those six keys carries a built-in list. When no rules layer sets one it stays empty, and the difficulty slot is used to index it anyway, reading storage that was never allocated. Give each of them one entry per difficulty.
:::

:::caution[More than one human player shifts the slot]
Outside a campaign, a computer house whose slot is not already 0 drops one slot when the session holds more than one human player and [`CompEasyBonus=yes`](/keys/compeasybonus/). The list index and the difficulty flag that is tested both move with it, so a Medium game with two human players reads entry 0 of each list and tests each trigger's Hard flag.
:::

## Defensive teams and the enemy

A trigger counts as defensive when its first TeamType is [`IsBaseDefense=yes`](/keys/isbasedefense/#scope-teamtype) and its second is either absent or defensive as well.

A house is restricted to defensive triggers — that is, a trigger that is not defensive is rejected outright — while either of these holds:

- the house has no enemy;
- **All of:** [`UseMinDefenseRule=yes`](/keys/usemindefenserule/), and the house's live defensive team count is below [`MinimumAIDefensiveTeams`](/keys/minimumaidefensiveteams/).

:::caution[The trigger's own base defense field decides nothing]
The base defense field in the trigger record is stored and written back out, but no decision reads it. Whether a trigger is treated as defensive is settled entirely by the `IsBaseDefense` setting of the TeamTypes it names.
:::

A house's enemy is the house it holds the most anger toward, and [damage is what moves that record](/systems/base-attacked/#anger-and-the-declared-enemy). Outside a campaign a computer house left without an enemy also picks one on a countdown; nothing does that in a campaign, so a campaign house has no enemy until combat gives it one, and until then only defensive triggers can pass at all.

## Conditions

The condition types themselves are listed with the [trigger record](/mapping/ai-triggers/). What each one measures at runtime is this:

- The two owning conditions count how many live objects of the record's condition object type the enemy, or the owning house, currently holds, and compare that count against the record's number with the record's comparator. A condition object that resolved to nothing counts as zero, so an unrecognized ID makes the comparison read `0`.
- The yellow power condition ignores the record's number and comparator entirely and tests whether the enemy's power output minus its drain is below a hard-coded `100`. The red power condition tests the same difference against `0`.
- The money condition compares the enemy's spendable credits against the record's number.

Every condition except the owning-house one reads the enemy, so a house without an enemy can only ever pass a defensive trigger that either has no condition or asks about its own holdings.

## The weighted draw

Every trigger that survives the gates is added to a distribution carrying its current weight, and one entry is drawn from that distribution. The three weights in a trigger record are truncated to whole numbers as the record is parsed, and the current weight is truncated again as it is added, so a trigger whose weight has fallen below `1` is added to the distribution but can never be drawn from it.

## The track record

Each trigger carries a current weight held between a minimum and a maximum of its own, and two counters: the runs it has had and the runs that succeeded. The counters start at zero and all three weights at `1` unless the record supplies them.

When a team dies, every trigger whose **first** TeamType is that team's type records an outcome — a success when the team reached the [Success](/mapping/missions/tmission-success/) team mission at some point in its script, a failure otherwise.

Both outcomes move the weight the same way: a history term plus a flat delta, added to the current weight. The table sets the two paths side by side. The successes and runs in the history column are the figures standing before this outcome, because the counters are stepped only afterward; the difference to read off the table is that the coefficient scales the failure path alone.

| Outcome | History term added | Delta added | Counters afterward |
| --- | --- | --- | --- |
| Success | The successes so far minus half the runs so far, held at no less than zero | [`AITriggerSuccessWeightDelta`](/keys/aitriggersuccessweightdelta/) | One run, one success |
| Failure | That same successes-minus-half-runs figure taken before any floor, scaled by [`AITriggerTrackRecordCoefficient`](/keys/aitriggertrackrecordcoefficient/), then held at no more than zero | [`AITriggerFailureWeightDelta`](/keys/aitriggerfailureweightdelta/) | One run |

Either result is clamped back into the trigger's own minimum and maximum. The history term is zero at exactly a one-in-two success rate and grows with the number of runs, so a long record moves the weight further than a short one does.

:::caution[A team that never runs Success records a failure]
Nothing but the Success team mission raises the flag the teardown reads. A team that fights well and dies before reaching that mission records a failure; so does a team that dissolves for want of members, and so does the defensive team the census culls.
:::

The recording is keyed on the first TeamType and nothing else. It does not check which trigger raised the team, or which house owns it, so every trigger in the scenario that names that TeamType first is credited with the same outcome, and a team the scenario placed by hand moves those records too. A trigger that only ever appears as somebody's second team never has its own record moved at all.

## From suggestion to team

Before the suggestion is returned the house's teams are scanned once more. If any team it owns is reforming, or is not under way, and that team's TeamType appears anywhere in the suggestion, the whole suggestion is discarded — the other team with it.

Every TeamType that survives is marked as autocreated, permanently for the rest of the session. [`Autocreate=yes`](/keys/autocreate/) sets the same mark from the start, but nothing in the selection above reads it: it is a recruitment setting, and being suggested applies it whether the section asked for it or not.

One team is then created from each surviving TeamType, subject to a second count test that is not the same as the eligibility test above. In a campaign game it compares `Max` against the teams of that type alive anywhere in the scenario rather than against the teams this house owns, so a campaign house can clear the eligibility test and still create nothing.

## Recruitment

A team fills its TaskForce by taking objects its house already owns. It looks for them on a logic pass while all of this holds:

- **Any of:**
  - the team is not under way: it has not been started yet, or it has since lost enough members to be sent back to regroup;
  - **All of:** it is short of full strength, and its TeamType is [`Reinforce=yes`](/keys/reinforce/).
- **Any of:**
  - the team belongs to a computer house;
  - the team has neither reached full strength nor been set in motion at any point since it was created.

The second group is what closes recruitment on a human house's teams, and it closes it for good. Reaching full strength once is enough, and so is starting once; losing members afterward does not reopen it, and neither does `Reinforce=yes`.

Each pass fills at most one place in each TaskForce slot that is still short — except that a recruited transport brings its passengers with it, which can fill several places at once.

Candidates are ranked by distance from the cell of the TeamType's [`Waypoint`](/keys/waypoint/), or from the team's own center when it has no waypoint, and the nearest one that may join is taken. An object is considered at all under any of these:

- the team's group is `-2`;
- the object carries the team's group;
- the TeamType is [`Recruiter=yes`](/keys/recruiter/).

A candidate outside the group is pushed 50 cells further away in the ranking before the nearest is chosen. The team's group is the TeamType's [`Group`](/keys/group/#scope-teamtype), which falls back to the TaskForce's [`Group`](/keys/group/#scope-taskforce) while the TeamType leaves its own at `-1`, and every member's group is overwritten with it as the member joins.

A considered candidate then has to clear all of these, in this order:

1. It is not already on a team of this same TeamType.
2. It is alive, it is not in radio contact, and its house is the team's house.
3. Its current mission is one that [allows recruitment](/keys/recruitable/).
4. Its recruitable state for this kind of team is still set, as below.
5. If it is already on another team, that team's [`Priority`](/keys/priority/#scope-teamtype) is strictly below this team's.
6. It is not an aircraft that carries a weapon and has no ammunition.
7. Its type matches one of the TaskForce's member lines, and that line still has room.

Which recruitable state step 4 tests depends on the autocreate mark: an unmarked TeamType reads the object's ordinary team-recruitable state and ignores its autocreate-recruitable state, and a marked one does the reverse. Both start set on every object. Joining a team overwrites the object's autocreate-recruitable state with the TeamType's [`AreTeamMembersRecruitable`](/keys/areteammembersrecruitable/), so a team declared `AreTeamMembersRecruitable=no` locks its members away from every autocreated team that comes after it.

A team left with no members dissolves at once when it has already reached full strength or been set in motion. Outside a campaign it also dissolves [`DissolveUnfilledTeamDelay`](/keys/dissolveunfilledteamdelay/) frames after it was created whether or not anybody ever joined it, which is the ordinary end of a team whose TaskForce the house could not fill.

## Production demand

Structures are chosen by [the base planner](/systems/ai-base-building/). Vehicles, infantry and aircraft are each chosen by their own routine, and the three share one shape — though the vehicle routine first considers replacing a lost harvester and can spend its turn on that. A routine that already has a type selected and unbuilt does nothing further.

1. **Demand.** A team of this house contributes to the tally under either of these:
   - **All of:** its TeamType is `Reinforce=yes`, and it is short of full strength;
   - **All of:** it is not forced active — a mark only a reinforcement group carries — and it has neither reached full strength nor been set in motion.

   Each unfilled place in a contributing team's TaskForce for a type of the routine's own kind adds one to that type's demand, and the creation frame of the earliest contributing team is kept per type.

2. **Supply.** Every object of the house that is recruitable — out of limbo, on no team, on a mission that allows recruitment and not held back by its own recruitable state — takes one off its type's demand.
3. **Affordable.** A type is a candidate while its demand is still above zero, the house can build it, and its cost is within the house's spendable credits.
4. **Choice.** With probability [`FillEarliestTeamProbability`](/keys/fillearliestteamprobability/) percent the house builds the type wanted by the oldest waiting team; otherwise it draws uniformly from the candidate list.

The uniform draw is not restricted to the types in most demand. The candidate list is emptied only when a type whose demand exceeds every demand seen so far is reached, so it holds every candidate found since the last such type, including candidates with less demand. Which candidates those are depends on the order the types are registered in.

The tally is sized from the current InfantryType, UnitType or AircraftType list. A team or
runtime object whose type position lies outside that list is ignored instead of indexing
unrelated storage.

## Parsed settings without effect

A TeamType's [`Prebuild`](/keys/prebuild/) and [`LooseRecruit`](/keys/looserecruit/) are read into flags no decision consults, and so is [its own `TechLevel`](/keys/techlevel/#scope-teamtype) — a trigger's tech level requirement comes from its teams' TaskForce members instead. [`AutocreateTime`](/keys/autocreatetime/) in `[AI]` is read into the rules and reaches only a disabled block.
