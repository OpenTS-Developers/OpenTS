---
title: Base defense response
summary: "Answers damage to a base by raising the owner's anger toward the attacker and calling a computer house's armed infantry and vehicles back to fight it."
category: ai-teams
keys:
  - AIHateDelays
  - Armor
  - AvoidThreats
  - BaseDefenseDelay
  - ComputerBaseDefenseResponse
  - Cost
  - IsBaseDefense
  - MovementZone
  - Priority
  - Recruitable
  - Strength
  - SuspendDelay
  - SuspendPriority
  - ThreatAvoidanceCoefficient
  - ThreatPosed
  - ToProtect
  - Verses
related:
  - type: system
    id: target-selection
  - type: system
    id: ai-team-production
  - type: enum
    id: MissionType
---

Damage moves three separate mechanisms. **Anger** is a running figure each house keeps against every other house, and a hit raises the anger the damaged object's house holds toward whoever caused it. A house treats whichever house it holds the most anger toward as its declared enemy. Every house's threat map records where each object stands, weighted by how dangerous its type is. Only a computer house runs the third: a call-up that empties its teams and sends armed infantry and vehicles back to the object that was hit.

That call-up is expressed through teams and the definitions they are built from. A **team** is a group of infantry, vehicles and aircraft that one house owns and that acts together. A **TeamType** is the scenario section a team is built from, and it holds the settings a team is measured by. [AI triggers and team production](/systems/ai-team-production/#teamtypes-and-ai-triggers-in-brief) introduces both entities in full; the individual settings this page turns on are defined where they are used.

## What counts as an attack

Two damage paths reach the call-up, and each hands it the object that caused the damage.

### Damage to a structure

A building damaged by another object stamps the current frame on its house and runs the call-up. The exception is a building that names an [`UndeploysInto`](/keys/undeploysinto/) vehicle and is not a construction yard: that structure counts as a deployed vehicle rather than as part of the base. Both happen before any strength is lost and before the branches that end the damage routine early. A hit absorbed entirely therefore still counts as an attack on the base, whether a laser fence stopped it, an immune bridge repair hut ignored it, or a firestorm wall drained it off the firestorm defense's own charge.

### A protected object

[`ToProtect=yes`](/keys/toprotect/) puts the same call-up on the type's runtime instances, for a computer house only. Any protected object of such a house is answered the same way, and a protected structure takes this call-up on top of the one every damaged structure already runs.

:::caution[A protected object calls for help only on ordinary damage]
The damage result selects which branch of the damage routine runs, and the call-up sits on one branch alone. That branch is the hit that landed damage and changed nothing else about the object's condition. A `ToProtect=yes` object therefore calls for help when **none of** the following describes the hit:

- it registered no damage at all;
- it took the object across the half-strength line;
- it took the object across the condition red line;
- it destroyed the object.

So a protected object stays silent on the hit that halves it, on the hit that takes it into the red, and on the killing blow. A protected structure is unaffected by this, because it already calls for help on every hit from an attacking object whatever the result.
:::

### What the house announces

The announcement is a separate mechanism from the call-up. It is reached under **all of** the following:

- the damage had a source;
- some damage actually registered;
- the damaged building does not set [`Insignificant=yes`](/keys/insignificant/);
- the damaged building is not a deployed vehicle.

What is then said depends on who is watching. A house under player control submits a radar event at the building's cell and speaks the base-under-attack line. A house allied to the local player speaks the ally-attacked line instead. Outside a campaign, that line also requires the attacked house's own country not to set [`MultiplayPassive=yes`](/keys/multiplaypassive/). [`SpeakDelay`](/keys/speakdelay/) holds off both lines and the radar event with them.

Every tag attached to a house then springs its attacked event, whichever house owns the building, and `SpeakDelay` does not hold that off.

### The attacked state

A house is in its attacked state while the frame stamped by the most recent hit on one of its structures lies within the previous minute. It leaves the state a minute after that hit. Two decisions read it. While the state holds, a house never reports a fire-sale urgency. That is the judgment that a house still holding buildings, but none of them able to produce anything, should sell the whole base off at once. The state also stops a computer-owned aircraft on Guard from looking for enemy vehicles caught outside any base.

The state is entered and left by the expert AI, the computer's periodic decision pass. That pass re-arms itself with 7 to 7.5 seconds each time it runs, and it is also [what fires a computer house's superweapons](/systems/superweapons/#the-computers-use). The stamp starts at frame zero, so every computer house that runs the pass begins the match in the attacked state and stays there for the first minute, before anything has been attacked. A `MultiplayPassive=yes` computer house never runs the pass and so never enters the state at all.

## Anger and the declared enemy

Every house holds one anger figure for every other house, and never one for itself. Its declared enemy is whichever house it holds the most anger toward.

### What raises it

Every damage event with a source raises the anger the damaged object's house holds against the attacker's house. The amount is the damaged type's build cost, scaled by the fraction of its [`Strength`](/keys/strength/) the hit removed and truncated to a whole number. For a structure the build cost is its [`Cost`](/keys/cost/), less any free unit it comes with and less any aircraft share bundled into it. The figure uses the damage that was applied, after armor, after distance falloff, and after a killing blow has been cut back to the strength that was left. A hit worth less than one whole unit therefore adds nothing at all. Healing is negative damage, so being repaired by a hostile healer takes anger back off.

This runs for every house, computer or human, allied or not, and it runs on damage to any object, not just to a structure.

### How the enemy is chosen

The whole ledger is re-read on every one of those events. The house holding the highest figure becomes the declared enemy, provided it is neither defeated nor a current ally. A tie goes to whichever house was created first. A figure of zero never wins, and when nothing qualifies the declared enemy is cleared outright.

Defeated houses and allies are excluded from the choice but not from the ledger. Their figures are kept, and they count again if the exclusion lifts. Allying with a house zeroes the anger held against it and drops it as the declared enemy. Breaking an alliance adds one point of anger and rebuilds both houses' threat maps. A computer house also clears the record of a declared enemy that has been defeated.

### Decay

Every hundredth frame, each figure above one is reduced by one.

:::caution[Anger decays to one, never to zero]
The step is skipped for a figure already at one. Nothing therefore leaves the ledger through decay alone, and a house that landed a single hit long ago remains a candidate forever. Decay also does not re-run the choice, so a house keeps its declared enemy until the next damage event, alliance change or defeat check moves it.
:::

### Picking a first enemy

A computer house adds one point of anger against another house under **all of** the following, tested in this order:

- its [`AIHateDelays`](/keys/aihatedelays/) countdown has run out, which is seeded once as a multiplayer session is set up;
- it holds no declared enemy;
- the game is not a campaign;
- its country is not `MultiplayPassive=yes`;
- it has a base center, which it gains as soon as one of its buildings is finished.

The point goes to whichever other house has the nearest base center, counting only houses that are neither defeated nor `MultiplayPassive=yes`. The pick is not filtered by alliance, so picking a current ally raises a figure the choice above will still refuse to act on.

:::caution[A campaign house has no enemy until it is hurt]
A campaign house is never given a first enemy: the expert AI reads the countdown, then refuses the pick outright in a campaign. A campaign house's record stays empty until something damages it or one of its objects. While it is empty, [only defensive AI triggers can spring](/systems/ai-team-production/#defensive-teams-and-the-enemy), and the ion cannon, multi-missile, chemical missile, hunter-seeker and drop-pod routines all stand down.
:::

### What the declared enemy drives

- Whether a non-defensive [AI trigger](/systems/ai-team-production/#which-triggers-are-eligible) may be considered at all, and the house every trigger condition but one measures.
- Which house the ion cannon rates objects from, and which house's buildings the multi-missile and chemical missile choose between.
- Whether the hunter seeker and the computer's drop pods fire.
- The preferred house for the two team missions that attack or move to a building with a named property.
- [`EnemyHouseThreatBonus`](/keys/enemyhousethreatbonus/) in target scoring, and the clamp a house ordered to hunt everything applies to anything outside the enemy's ownership. Both belong to [target selection](/systems/target-selection/#the-threat-score).

## The threat map

Every house keeps one figure per map region, and a region is a block of 4 by 4 cells. An object contributes its type's [`ThreatPosed`](/keys/threatposed/) to every house's map except its owner's.

Each contribution is stamped across a 3 by 3 block of regions. The whole figure goes into the region the object stands in, half of it into the four regions beside that one, and a quarter into the four corners, each step discarding its remainder. Taking a contribution back runs the same stamp as a subtraction. Every region floors at zero, so one that has already bottomed out does not return what it was never charged, and the two operations are not exactly reversible.

### What adds and removes a contribution

Five events add a contribution or take one back. Because the stamp covers a 3 by 3 block of regions, each of them has to name one cell to center that block on. The second column below is that cell, not wherever the object happens to be standing. The last two rows are the ones that do not balance, and each is taken up by what follows the table. An ownership change stamps the contribution the wrong way round, and a visceroid merge is the one event here that adds a contribution nothing ever takes back.

| Event | Cell the stamp is centered on |
| --- | --- |
| The object is placed on the map | The cell it appears in |
| The object leaves the map | For infantry, vehicles and aircraft, the last cell recorded for it; for anything else, its own cell |
| A ground object finishes moving into a cell | Credited at the new cell and debited at the old one, and only when the two fall in different regions |
| The object changes owner | Withdrawn under the previous owner's identity and re-stamped under the new owner's, at the same cell: the previous owner's own map gains the contribution and the new owner's loses it |
| A small visceroid merges into a large one | Credited at the enlarged unit's last recorded cell, with no matching debit |

The owner exemption is narrower than it looks. A house skips its own objects always. It skips an ally's objects only while that house is under human control, so a computer house records its allies' objects as threats along with everybody else's.

:::caution[An aircraft's contribution does not follow it]
Only the ground-movement step moves a contribution between regions, and aircraft do not run it. An aircraft's contribution therefore stays in the region it was placed in for as long as it flies. Landing updates the cell it will eventually be debited at, without moving the contribution there. An aircraft that flies far from where it started therefore leaves its threat behind and takes its debit somewhere else again.
:::

### Rebuilding the map

A house rebuilds its entire map in two situations only: when it forms an alliance, and when an alliance is broken, on both sides. Outside a campaign every house is allied with the neutral Special house once every object has been placed, so every house's map is rebuilt at scenario start. In a campaign those alliances are read before any object exists, and nothing rebuilds. The map is otherwise purely incremental.

:::caution[A rebuild records the house's own buildings as threats]
The rebuild walks every object in the match. It applies the ownership and alliance test to infantry, vehicles and aircraft, but the branch that handles everything else applies neither. Every building in the scenario is therefore stamped into the map, this house's own and its allies' included, while objects that have never stood on the ground are dropped from it entirely. A house that has just allied, or has just had an alliance broken, therefore reads a map that no longer matches the one its incremental accounting maintained. Its own base is among the most dangerous ground on that map.
:::

### What reads the map

- The multi-missile and the chemical missile each strike the enemy building standing in the highest-rated region.
- A team that has begun to move and has [fallen under strength](/systems/ai-team-execution/#the-state-flags) is sent to gather at one of its house's unarmed buildings. The building is ranked by distance multiplied by the region's figure plus one, and halved for a building that repairs vehicles.
- The least-threat and greatest-threat building properties a team script can ask for.
- The pathfinder reads it through [`ThreatAvoidanceCoefficient`](/keys/threatavoidancecoefficient/). A diagonal shortcut is refused where the region figure times the coefficient reaches `1`. A two-leg straight line counts each cell whose product reaches `0.01` as threatened and abandons the line once too many have accumulated. The hierarchical route planner prices a step from the regions it spans.

:::caution[Threat avoidance is off until a coefficient is written]
Every type's coefficient starts at `0`. A team's [`AvoidThreats=yes`](/keys/avoidthreats/) raises it to `1` for that team's members and is the only thing that raises it without a rules edit. At zero every product the pathfinder tests is zero, so no cell is ever counted as threatened and no shortcut is ever refused, however high the region figures climb.
:::

## Calling defenders back

### When the call-up is refused

The call-up ends before doing anything when any of the following holds, tested in this order. The whole table is about the damaged object and its attacker, not about the defenders, so nothing here can be fixed by giving the house better troops. The last row is the one that leaves a repeat attack by the same attacker unanswered.

| Refused when | Detail |
| --- | --- |
| The attacker is an ally of the damaged object's house | |
| The house is under human control | Player control in a campaign, and a human player otherwise |
| The damaged object has a primary weapon | Campaign games only: a base defense structure defends itself and pulls nobody back |
| The attacker is neither an infantry nor a vehicle | Aircraft and buildings never provoke a response |
| The damaged type sets `Insignificant=yes` | |
| The attacker's cooldown from an earlier response is still running | Below |

### The strength budget

The response is sized once, at the attacker's [`ThreatPosed`](/keys/threatposed/) multiplied by [`ComputerBaseDefenseResponse`](/keys/computerbasedefenseresponse/) in `[AI]`. An attacker whose type leaves `ThreatPosed` at zero produces a budget of zero and calls up nobody, whatever the multiplier is.

Anything already firing at the attacker is counted off that budget rather than added to the shortlist, at its own `ThreatPosed`. At the stock multiplier of `3`, one such defender whose own `ThreatPosed` is three times the attacker's takes the budget to exactly zero, cancelling the response by itself. Exactly zero rather than below it is the distinction [the cooldown](#the-attackers-cooldown) turns on.

### Teams are emptied first

Before any candidate is examined, every team the house owns is measured against one threshold. A TeamType's [`Priority`](/keys/priority/#scope-teamtype) is the figure that settles which of two teams wins a member they both want. [`SuspendPriority`](/keys/suspendpriority/) in `[General]` is the level a team's own priority has to reach to survive a call-up. Every team below it has all of its members removed and is suspended for [`SuspendDelay`](/keys/suspenddelay/) minutes, meaning the team takes no logic turn of any kind until that timer runs out.

The team object outlives the suspension, but it does not come back. A team holds a mark set the first time it ever reached full strength, or the first time it was set in motion. Full strength here is the member count its TaskForce asks for, which is a roster figure and nothing to do with health. The mark is never cleared afterward. When the suspension expires, a team holding that mark is deleted outright for having no members. A team that never earned it gets one pass at recruiting instead. It then falls to [the ordinary rule](/systems/ai-team-production/#recruitment) that dissolves a team still empty a set number of frames after it was created, which applies outside a campaign only. Either way the members freed by the emptying have no team for the rest of the same call-up, which is what makes them eligible below.

:::caution[At the engine defaults every base attack empties every team]
`SuspendPriority` defaults to `20` and a TeamType's `Priority` defaults to `7`, so a TeamType that does not state a priority sits below the threshold. With neither key written, every team the house owns is stripped of its members every time a call-up actually runs, and stays empty for two minutes. That covers attack teams and scenario-placed teams alike. Lower `SuspendPriority` to the lowest priority that should survive, or give those TeamTypes a priority at or above it.
:::

The four rules settings the response reads, in the two sections that hold them:

```ini title="rules.ini"
[AI]
ComputerBaseDefenseResponse=4 ; scales the budget the attacker's ThreatPosed is multiplied by

[General]
BaseDefenseDelay=1            ; minutes before the same attacker can be answered again
SuspendPriority=10            ; a team below this priority is emptied and suspended
SuspendDelay=3                ; minutes an emptied team takes no logic turn
```

### Which objects qualify

Infantry are walked first, then vehicles; aircraft and buildings are never candidates. Both passes stop the moment the budget is used up. Each candidate is rejected on the first of these that matches, so a candidate that survives the whole table can reach the damaged object and hurt its attacker once it arrives. The rows fall into two halves. The first five ask whether the candidate is available at all. The last three ask about this particular attack: the attacker's armor, the ground in between, and whether the candidate has anything left to bring. No amount of strength standing nearby can talk around any of those three.

| Rejected when | Detail |
| --- | --- |
| It is inactive, or belongs to another house | |
| It is on a team whose TeamType is not [`IsBaseDefense=yes`](/keys/isbasedefense/#scope-teamtype) | A team member that is already a base defender may be taken; any other team member may not |
| Its recruitable state, or its autocreate-recruitable state, is off | Both start set on every object |
| It has no primary weapon | |
| Its current mission sets [`Recruitable=no`](/keys/recruitable/) | Campaign games only |
| Its primary warhead's [`Verses`](/keys/verses/) percentage against the attacker's [`Armor`](/keys/armor/) is exactly `0%` | |
| It is not in the same movement zone as the damaged object | The two destinations are compared, not the two positions, using the candidate's [`MovementZone`](/keys/movementzone/) |
| It cannot bring any strength to bear | Below |

A candidate brings nothing — a rating of zero, which drops it — under **any of** the following, tested in this order:

- it is already attacking something that has a weapon;
- it is on a team whose TeamType is not `IsBaseDefense=yes`;
- its mission is Harvest;
- its own `ThreatPosed` is zero.

Otherwise its rating is `ThreatPosed` times `1024`. That figure stands as it is when the attacker is already inside the candidate's weapon range. When it is not, the figure is divided by the distance still to cover, measured in multiples of the candidate's own top speed, and never falls below `1`. A candidate already targeting the attacker rates minus its `ThreatPosed` instead, and that negative rating is what the budget is reduced by.

:::caution[The already-engaged multiplier is asymmetric, and can cancel the response]
Both passes apply a multiplier meant to favor a defender already sent to protect this object, a hundredfold for infantry and tenfold for vehicles. The vehicle pass applies it only to a positive rating. The infantry pass applies it before the sign is tested, so a negative rating is multiplied too. Where the condition holds, an infantry already firing at the attacker takes a hundred times its own `ThreatPosed` off the budget in one step, enough to end the response for every other candidate. The condition both passes test belongs to the damaged object rather than to the candidate, so within one call-up the multiplier applies to every candidate or to none.
:::

### The shortlist and the orders

The shortlist holds six entries, shared between the two passes and filled in order. Once it is full, a stronger candidate is meant to replace the weakest entry. The entry to replace is found by matching the stored ratings against a running minimum kept beside the list. That running minimum starts at zero and is updated only by a candidate examined after the list filled, so for one candidate it matches nothing the list holds.

The table walks eight candidates through the list with one illustrative set of ratings. The first six fill it and never touch the running minimum. The last two run into the replacement rule.

| Candidate | Its rating | The shortlist after it | The running minimum after it |
| --- | --- | --- | --- |
| 1 | 30 | 30 | 0 |
| 2 | 25 | 30, 25 | 0 |
| 3 | 40 | 30, 25, 40 | 0 |
| 4 | 20 | 30, 25, 40, 20 | 0 |
| 5 | 35 | 30, 25, 40, 20, 35 | 0 |
| 6 | 45 | 30, 25, 40, 20, 35, 45 (full) | 0 |
| 7 | 50 | unchanged: it beats the running minimum, but no entry holds `0`, so nothing is replaced | 20 |
| 8 | 28 | 30, 25, 40, **28**, 35, 45 | 25 |

Candidate 7 is the one that pays for repairing the running minimum. It clears the minimum of zero, finds no entry to overwrite, and its whole effect is to leave the minimum standing at the list's true weakest figure. Candidate 8 is the first that displaces anything. Because infantry are walked before vehicles, six qualifying infantry therefore cost the first vehicle examined its place, and every vehicle after that one competes normally. That is how vehicles come to displace infantry that were admitted without being ranked against anything.

One further case keeps the list closed for longer. A candidate rated below the list's own weakest entry leaves the running minimum at its own rating rather than the list's. That again matches nothing, so the candidate after it is dropped as well. The running minimum settles onto the list's weakest figure only once a candidate rated at or above that figure has been examined. When more than one entry holds the running minimum, all of them are overwritten with the same candidate, so the list can hold one object several times.

The list is sorted from strongest to weakest and worked through in that order. Each defender takes the [Rescue](/reference/enums/mission/) mission on a 66 percent chance and Area Guard otherwise; a member of an `IsBaseDefense=yes` team always takes Area Guard. Either way the damaged object is recorded as what the defender was sent to protect, and the attacker is assigned as its target.

A defender on Rescue engages whatever [it finds near the damaged object](/systems/target-selection/#when-an-object-scans) and then heads for a cell its house nominates, settling into Area Guard when it arrives. An object with no offensive value at all is sent to the core of the base, a random point within one base radius of the base center. Everything else is sent to one of the four edge zones at random. An edge zone is the ground lying in one compass quadrant of the base, between one and two base radii out from that same center.

Each defender's `ThreatPosed` is subtracted from the budget after it has already been given its orders, so the first defender is always sent even when it alone covers the whole budget.

### The attacker's cooldown

Once the dispatched defenders have covered the budget, a countdown of [`BaseDefenseDelay`](/keys/basedefensedelay/) minutes in `[General]` starts. It is stored on the attacker rather than on the defending house, so one satisfied call-up stops every house on the map from answering that same attacker until it expires.

A call-up that ran out of candidates before covering the budget sets no cooldown and runs again on the attacker's next hit. A call-up canceled by defenders already fighting the attacker sets it only when their combined ratings pushed the budget below zero. One canceled to exactly zero, the case of the single defender in [the strength budget](#the-strength-budget) above, sets nothing. Neither does an attacker whose own rating is zero.

## Campaign and skirmish differences

Five of the decisions above are settled differently by game type, and they are gathered here so that a page written mostly in one voice does not have to be re-read for the other. A campaign holds the response back. No first enemy is ever picked, an armed building keeps its own defenders instead of calling for help, and a mission that forbids recruitment is honored where the other game types ignore it. Breaking an alliance is the one rule a campaign relaxes rather than tightens.

| Behavior | Campaign | Skirmish and multiplayer |
| --- | --- | --- |
| Picking a first enemy | Never happens | The nearest non-passive, undefeated house, once the countdown expires |
| An armed building answering its own attack | Refuses the call-up and defends itself alone | Calls for help like any other structure |
| A candidate whose mission sets `Recruitable=no` | Rejected | Accepted; the mission's setting is not read |
| Which houses count as human | The house under player control | Every house flagged as human |
| Breaking an alliance | Always proceeds | Requires that neither country is `MultiplayPassive=yes` |

## Settings and state without effect

[`Whiner=yes`](/keys/whiner/) on a TeamType is read into a flag whose only test can never be reached. The damage routine sends a team member's damage to its team and everything else down a second branch. The flag is tested on that second branch, behind a test for a team that the branch itself has already excluded, so nothing that holds a team ever reaches it. A team that is hit before it began to move regroups instead, and one that is hit while moving turns on its attacker.

A house also sets a flag for having been alerted to an enemy. It is set for every computer house that builds a base, or whose [`IQ`](/keys/iq/) reaches the production level, and set again by the [Autocreate Begins...](/mapping/actions/taction-autocreate/) trigger action. The team-creation block that would have read it is disabled, and the live pass that replaced it ignores the parameter it is handed, so there is no alerted-team behavior. [`AutocreateTime`](/keys/autocreatetime/) reaches [the same disabled block](/systems/ai-team-production/#parsed-settings-without-effect).

Two pieces of state are written and never read. Alongside the frame stamp that drives [the attacked state](#the-attacked-state), a damaged structure records which house the attacker belongs to, and nothing reads that record. The engine also declares a further house state for an enemy closing in on the base, which nothing ever assigns and nothing ever tests.
