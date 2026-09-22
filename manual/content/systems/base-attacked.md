---
title: Base defense response
summary: "How damage raises one house's anger at another, what each house's threat map records, and how a computer house calls armed infantry and vehicles back to defend."
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

Damage to an object has two effects. It raises the anger the object's owner holds toward the attacker's house, and each house treats the house it is angriest at as its **declared enemy**. If a computer house owns the object, the hit can also start a **call-up**, which empties the house's low-priority teams and sends armed infantry and vehicles back to fight the attacker.

Each house also keeps a **threat map** of where other houses' objects stand, weighted by each type's [`ThreatPosed`](/keys/threatposed/). Damage does not change the map. The call-up is sized from the same `ThreatPosed` ratings, and the computer's missiles, its teams and the pathfinder read the map.

The call-up works through teams. A **team** is a group of infantry, vehicles and aircraft that one house owns and that acts together. A **TeamType** is the scenario section a team is built from. [AI triggers and team production](/systems/ai-team-production/#teamtypes-and-ai-triggers-in-brief) introduces both. This page defines each TeamType setting it uses where that setting appears.

## What counts as an attack

Two kinds of hit run the call-up: any hit on a structure, and an ordinary hit on a `ToProtect=yes` object. Both pass the call-up the object that caused the damage.

### Damage to a structure

A structure runs the call-up whenever another object hits it. The call-up runs before the structure loses any strength, so a hit that does no damage still counts as an attack on the base. This includes a hit a laser fence ignores, a hit on an immune bridge repair hut, and a hit a firestorm wall absorbs by draining the Firestorm Defense charge. The same hit starts the owner's [attacked state](#the-attacked-state).

A structure with an [`UndeploysInto`](/keys/undeploysinto/) vehicle counts as a deployed vehicle, not as part of the base, and never runs the call-up. A construction yard is the exception and counts as part of the base.

### A protected object

[`ToProtect=yes`](/keys/toprotect/) makes objects of that type run the call-up when a computer house owns them. A protected structure runs it a second time, on top of the structure call-up above. The second run is refused if the first one started [the attacker's cooldown](#the-attackers-cooldown).

:::caution[A protected object calls for help only on ordinary damage]
A `ToProtect=yes` object calls for help only on a hit that damages it without changing its condition. It stays silent on a hit that:

- does no damage;
- takes it below half strength;
- takes it below the [`ConditionRed`](/keys/conditionred/) threshold;
- destroys it.

A protected structure still calls for help on those hits through the structure rule.
:::

### What the house announces

A hit on a structure can also produce a spoken warning and spring the owner's attacked trigger event. This happens when **all of** the following hold:

- the hit came from another object;
- the hit did damage, so a hit that was absorbed entirely does not count;
- the structure does not set [`Insignificant=yes`](/keys/insignificant/);
- the structure is not a deployed vehicle.

The warning depends on the owner:

- If the local player controls the owner, the radar marks the structure's cell and the base-under-attack line plays.
- If the owner is allied to the local player, the ally-attacked line plays instead. Outside a campaign, this line also requires that the owner's country does not set [`MultiplayPassive=yes`](/keys/multiplaypassive/).

After either line, the owner plays no further warning and marks no further radar cell for [`SpeakDelay`](/keys/speakdelay/) minutes.

Every such hit also springs the attacked event on each trigger tag attached to the owner, whether a computer or a human controls it. `SpeakDelay` does not delay the event.

### The attacked state

The expert AI is a computer house's periodic decision pass. It runs every 7 to 7.5 seconds and also [fires the computer's superweapons](/systems/superweapons/#the-computers-use). A computer house whose country sets `MultiplayPassive=yes` never runs it.

A computer house is in its attacked state for one minute after the most recent hit on one of its structures. The state starts and ends only when the house's expert AI runs, so each change can come up to 7.5 seconds late. A house that never runs the expert AI never enters the state.

Every computer house that runs the expert AI is in the attacked state for the first minute of the match, before anything attacks it, because the time of the last hit starts at zero.

Two decisions read the state:

- A house in the attacked state never orders a fire sale. A fire sale sells the whole base when the house owns structures but none of them can produce anything.
- A computer-owned aircraft on Guard does not look for enemy vehicles caught far from their owner's base.

## Anger and the declared enemy

Each house holds one anger figure for every other house, and none for itself. Its declared enemy is the house it holds the most anger toward, among the houses it is allowed to choose.

### What raises it

Every hit from another house's object raises the anger the damaged object's owner holds toward that house. This applies to every house, computer or human, allied or not, and to damage to any object, not only to structures.

The increase is the damaged type's build cost multiplied by the fraction of its [`Strength`](/keys/strength/) the hit removed, with the fraction dropped. For a structure, the build cost is its [`Cost`](/keys/cost/) less the cost of any free unit it comes with, and less the average price of the aircraft it comes with on a helipad. For example, a vehicle costing `1000` with `Strength=400` that loses `100` strength raises anger by `250`.

The damage counted is the damage applied after armor and distance falloff. On a killing blow it is cut back to the strength that was left. A hit worth less than one whole point adds nothing. Healing counts as negative damage, so being repaired by another house's object lowers the anger toward that house.

These hits add no anger:

- a hit that a laser fence ignores;
- a hit on an immune bridge repair hut;
- a hit that an active firestorm wall absorbs.

A hit on an [`Immune=yes`](/keys/immune/) object adds anger for its damage without the warhead's `Verses` against the object's `Armor` or the distance falloff.

A house also gains one point of anger toward a house it turns hostile to, whether or not the two were allied. Breaking an alliance and the [Make enemy](/mapping/actions/taction-make-enemy/) trigger action both do this. Outside a campaign, neither happens when either country sets `MultiplayPassive=yes`.

### How the enemy is chosen

A house re-runs the choice whenever damage, an alliance change or the first-enemy pick changes its anger. The house with the highest anger figure becomes the declared enemy, skipping defeated houses and current allies. A figure must be above zero to win. A tie goes to the house created first. When no house qualifies, the house has no declared enemy.

Alliances and defeats change the choice directly:

- Allying with a house zeroes the anger held toward it and drops it as the declared enemy.
- When a computer house's declared enemy is defeated, the computer house zeroes its anger toward that house. It then has no declared enemy until its anger next changes.

Apart from those two resets, the choice only skips defeated houses and allies. Their figures stay. An ally's figure, including anger from hits after the alliance formed, counts again once the alliance ends.

### Decay

Every 100 frames, each anger figure above `1` drops by `1`.

:::caution[Anger decays to one, never to zero]
A figure at `1` does not decay. A house that landed even one hit long ago stays a candidate for declared enemy for the rest of the match. Decay also does not re-run the choice, so a house keeps its declared enemy until the next damage, alliance change or defeat changes it.
:::

### Picking a first enemy

Outside a campaign, a computer house with no declared enemy picks one. On each run of the expert AI, it adds one point of anger toward another house when **all of** the following hold, tested in this order:

1. its [`AIHateDelays`](/keys/aihatedelays/) countdown has run out; the countdown is set once when a skirmish or multiplayer match starts;
2. it has no declared enemy;
3. the game is not a campaign;
4. its country does not set `MultiplayPassive=yes`;
5. it has a base center, which it has while at least one of its structures is on the map.

The point goes to the house whose base center is nearest, counting only houses that are not defeated and whose country does not set `MultiplayPassive=yes`. The pick ignores alliances. If the nearest house is an ally, the point goes to that ally, which the choice never accepts.

The point then re-runs the choice, so the eligible house with the most anger becomes the declared enemy, whichever house received the point. If no other eligible house has anger above zero, the house stays without a declared enemy while that ally is nearest. After a declared enemy is defeated, other houses often still hold anger, and the first point makes the angriest of them the enemy.

:::caution[A campaign house has no enemy until its anger rises]
A campaign house never picks a first enemy. It has no declared enemy until damage or a Make enemy trigger action raises its anger. Until then, [only defensive AI triggers can pass](/systems/ai-team-production/#defensive-teams-and-the-enemy), and the computer does not fire its ion cannon, multi-missile, chemical missile, hunter seeker or drop pods.
:::

### What the declared enemy drives

- Whether a non-defensive [AI trigger](/systems/ai-team-production/#which-triggers-are-eligible) can be considered at all. Every trigger condition except the one about the house's own holdings measures the declared enemy.
- Which house the computer's ion cannon rates targets from, and which house's structures its multi-missile and chemical missile choose between.
- Whether the computer fires its hunter seeker and drop pods. Outside a campaign, a computer house's hunter seeker hunts only the declared enemy's objects.
- The preferred house for the two team missions that attack or move to a structure with a named property.
- [`EnemyHouseThreatBonus`](/keys/enemyhousethreatbonus/) in target scoring, and the low score a house ordered to hunt everything gives to objects the declared enemy does not own. Both belong to [target selection](/systems/target-selection/#the-threat-score).

## The threat map

Each house keeps one threat figure for every region of the map. A region is a block of 4 by 4 cells. Every infantry, vehicle, aircraft and structure adds its type's [`ThreatPosed`](/keys/threatposed/) to the maps of the other houses, never to its owner's.

Allies are handled by who controls the house. A human-controlled house leaves its allies' objects off its map. A computer house counts its allies' objects as threats like everyone else's.

Each contribution is spread over a 3 by 3 block of regions around the object's region. The center region gets the full figure, the four regions beside it get half, and the four corner regions get a quarter, each with the fraction dropped. Removing a contribution subtracts the same shares.

A region never drops below zero. Any part of a subtraction that would take it below zero is lost, so after such a subtraction the map can end up higher than the objects on it account for.

### What adds and removes a contribution

Each event below adds or removes a contribution, spread around the cell in the second column. That cell is not always where the object currently stands. For infantry, vehicles and aircraft, the **recorded cell** is the last cell the object finished moving into on the ground, or the cell where an aircraft last landed.

| Event | Cell the contribution is spread around |
| --- | --- |
| An object is placed on the map | The cell it appears in |
| An object is taken off the map, for example when destroyed or loaded into a transport | For infantry, vehicles and aircraft, the recorded cell; for a structure, the cell it stands in |
| An infantry or vehicle finishes moving into a cell in a different region | Added around the new cell and removed around the previous one |
| An object changes owner | The same cell as when it is taken off the map. The previous owner's map gains the contribution, and the new owner's map loses it. |
| A small visceroid merges into a large one | Added around the large visceroid's recorded cell, with nothing removed |

The visceroid merge is the only event that does not balance. The large visceroid's contribution is added on top of the small visceroid it replaced, and only the large figure is removed later. The small visceroid's `ThreatPosed` therefore stays on the map where the merge happened.

:::caution[An aircraft's contribution does not follow it]
Only ground movement moves a contribution between regions, and aircraft do not use it. An aircraft's contribution stays in the region where it was placed for as long as it exists. Landing changes the recorded cell, where the contribution will eventually be removed, without moving the contribution there. An aircraft that lands far from where it was placed leaves its threat behind, and its removal subtracts from a different part of the map.
:::

### Rebuilding the map

A house clears its map and rebuilds it from every object in the match in two situations:

- it allies with another house;
- an alliance between it and another house is broken, in which case both houses rebuild.

Outside a campaign, every house allies with the neutral Special house when the match starts, after the map's objects are placed, so every house rebuilds its map then. In a campaign, alliances are set before any object exists, so no rebuild happens at the start. Otherwise the map changes only through the events in the table above.

:::caution[A rebuild counts the house's own structures as threats]
A rebuild applies the owner and ally exemptions to infantry, vehicles and aircraft only. It adds every structure in the match, including the house's own and its allies', and it leaves out aircraft that have never landed. It also counts objects that are off the map, such as passengers in a transport. A passenger that unloads after a rebuild then adds its contribution a second time. After a rebuild, the house's own structures with a `ThreatPosed` raise the figures around its base. Because every house rebuilds at the start of a skirmish or multiplayer match, this applies there from the start.
:::

### What reads the map

- The computer's multi-missile and chemical missile strike the declared enemy's structure that stands in the highest-rated region.
- A team that is under way and has [fallen under strength](/systems/ai-team-execution/#the-state-flags) regroups at one of its house's unarmed structures. Each structure is scored by its distance from the team multiplied by one more than its region's figure, and the score is halved for a structure that repairs vehicles. The lowest score wins.
- The least-threat and greatest-threat structure properties a team script can ask for.
- The pathfinder, through the moving object's [`ThreatAvoidanceCoefficient`](/keys/threatavoidancecoefficient/):
  - A diagonal shortcut is refused when the figure at the cell where it starts, multiplied by the coefficient, reaches `1`.
  - A straight-line shortcut counts each cell whose figure multiplied by the coefficient reaches `0.01` as threatened. Depending on the pass, one threatened cell or more than three abandon the shortcut.
  - The coarse corridor search of [route search](/systems/route-search/) adds the region figure, multiplied by the coefficient, to the price of each step.

:::caution[Threat avoidance is off until a coefficient is set]
Every type's `ThreatAvoidanceCoefficient` starts at `0`. At zero every product the pathfinder tests is zero, so no cell counts as threatened and no shortcut is refused, however high the region figures climb. A team's [`AvoidThreats=yes`](/keys/avoidthreats/) sets the coefficient to `1` for that team's members, and is the only way to raise it without a rules change.
:::

## Calling defenders back

### When the call-up is refused

The call-up does nothing when any of the following holds, tested in this order. Every condition concerns the damaged object and the attacker. None depends on which defenders the house has.

| Refused when | Detail |
| --- | --- |
| The attacker is an ally of the damaged object's owner | |
| A human controls the owner | In a campaign, the house under player control; otherwise any house a human plays |
| The damaged object has a primary weapon | Campaigns only. An armed structure or protected object calls no one back, even when its weapon cannot hit the attacker, such as an anti-aircraft structure hit by infantry. |
| The attacker is not an infantry or a vehicle | Aircraft and structures never provoke a response |
| The damaged type sets `Insignificant=yes` | |
| The attacker's cooldown from an earlier call-up is still running | See [the attacker's cooldown](#the-attackers-cooldown) |

A call-up that passes these checks [empties the house's low-priority teams](#teams-are-emptied-first) before it looks for defenders.

### The strength budget

The call-up looks for defenders worth the attacker's [`ThreatPosed`](/keys/threatposed/) multiplied by [`ComputerBaseDefenseResponse`](/keys/computerbasedefenseresponse/) in `[AI]`. That product is the **budget**. An attacker whose type has `ThreatPosed=0` gives a budget of zero, and no one is called back, whatever the multiplier is.

A qualifying object that is already targeting the attacker is not called back again. Its `ThreatPosed` is subtracted from the budget instead. Once the budget reaches zero or below, the search for defenders stops.

With `ComputerBaseDefenseResponse=3`, suppose one object is already targeting the attacker, and its `ThreatPosed` is three times the attacker's. It brings the budget to exactly zero, so no defender is called back. Whether the budget ends at exactly zero or below it decides whether [the cooldown](#the-attackers-cooldown) starts.

### Teams are emptied first

Before looking for defenders, the call-up empties every team of the house whose TeamType [`Priority`](/keys/priority/#scope-teamtype) is below [`SuspendPriority`](/keys/suspendpriority/) in `[General]`. Each emptied team is suspended for [`SuspendDelay`](/keys/suspenddelay/) minutes and does nothing until that time has passed. Its former members have no team for the rest of the call-up, which lets them qualify as defenders.

Teams are emptied on every call-up that passes the refusal checks, even when the budget is zero or no defender qualifies. A call-up that sets no cooldown runs again on the attacker's next hit and restarts the suspension. An attack that keeps hitting without starting a cooldown therefore keeps those teams empty and suspended.

What happens when the suspension ends depends on whether the team had started. A team has started once it has reached full strength or been flagged into action, and nothing clears that mark. Full strength here is the member count its TaskForce asks for, not hit points.

- A team that had started is deleted, because it has no members.
- A team that had not started resumes recruiting. Outside a campaign, [the ordinary rule](/systems/ai-team-production/#recruitment) dissolves it if it is still empty [`DissolveUnfilledTeamDelay`](/keys/dissolveunfilledteamdelay/) frames after it was created.

:::caution[With the default priorities, every call-up empties every team]
`SuspendPriority` defaults to `20` and a TeamType's `Priority` defaults to `7`. If neither is set, every call-up empties all of the house's teams, attack teams and scenario-placed teams alike, and suspends them for `SuspendDelay` minutes, two by default. To keep a team, raise its TeamType's `Priority` to at least `SuspendPriority`, or lower `SuspendPriority` to that team's priority or below.
:::

The call-up reads four settings from `rules.ini`. These are example values; the linked key pages give the defaults.

```ini title="rules.ini"
[AI]
ComputerBaseDefenseResponse=4 ; budget is the attacker's ThreatPosed times 4

[General]
BaseDefenseDelay=1            ; minutes before any house answers the same attacker again
SuspendPriority=10            ; teams with a Priority below 10 are emptied
SuspendDelay=3                ; minutes an emptied team stays suspended
```

### Which objects qualify

The call-up examines infantry first, then vehicles. Aircraft and structures are never called back. Both passes stop once the budget reaches zero or below. An object is rejected on the first of these that applies:

| Rejected when | Detail |
| --- | --- |
| It is inactive, or another house owns it | |
| It is on a team whose TeamType does not set [`IsBaseDefense=yes`](/keys/isbasedefense/#scope-teamtype) | A member of a base-defense team can be called back; a member of any other team cannot |
| Its recruitable state or its autocreate-recruitable state is off | Both start set; see [recruitment](/systems/ai-team-production/#recruitment) |
| It has no primary weapon | |
| Its current mission sets [`Recruitable=no`](/keys/recruitable/) | Campaigns only |
| Its weapon's warhead has a [`Verses`](/keys/verses/) value of exactly `0%` against the attacker's [`Armor`](/keys/armor/) | The primary weapon, or the elite weapon for an elite object |
| It is not in the same movement zone as the damaged object | Its destination is compared with the damaged object's, using its [`MovementZone`](/keys/movementzone/) |
| Its rating is zero | See below |

An object that passes these checks and is already targeting the attacker reduces [the budget](#the-strength-budget). Otherwise it gets a **rating**, which measures how much strength it can bring against the attacker. The rating is zero, and the object is dropped, when **any of** the following holds, tested in this order:

- it is targeting something else that has a primary weapon;
- its mission is Harvest;
- its `ThreatPosed` is `0`.

Otherwise its rating is its `ThreatPosed` multiplied by `1024` when the attacker is already within its primary weapon's range. When the attacker is out of range, the rating is divided by the distance beyond that range, measured in multiples of the object's top speed. The divisor is at least `1`, and the rating never drops below `1`. A closer or faster object therefore rates higher.

### The shortlist and the orders

The call-up keeps a shortlist of up to six candidates, shared between infantry and vehicles. The first six candidates fill it in the order they are examined, without being compared.

After the list is full, a candidate whose rating beats a **recorded low** replaces the entry whose rating equals the recorded low. The recorded low starts at zero and is updated only by candidates examined after the list filled. No entry holds zero, so the first replacement attempt always fails.

The table follows eight candidates through the list with example ratings.

| Candidate | Rating | Shortlist after it | Recorded low after it |
| --- | --- | --- | --- |
| 1 | 30 | 30 | 0 |
| 2 | 25 | 30, 25 | 0 |
| 3 | 40 | 30, 25, 40 | 0 |
| 4 | 20 | 30, 25, 40, 20 | 0 |
| 5 | 35 | 30, 25, 40, 20, 35 | 0 |
| 6 | 45 | 30, 25, 40, 20, 35, 45 (full) | 0 |
| 7 | 50 | unchanged: 50 beats the recorded low, but no entry holds `0`, so nothing is replaced | 20 |
| 8 | 28 | 30, 25, 40, **28**, 35, 45 | 25 |

The rule has three consequences:

- The first candidate examined after the list fills is always dropped, whatever its rating. It only sets the recorded low. Because infantry are examined first, when exactly six infantry qualify, the first qualifying vehicle is the one dropped. Later vehicles compete normally and can replace infantry.
- While the recorded low is below every entry, every candidate is dropped. This happens when the first candidate after the list fills is rated below all six, because that candidate sets the recorded low to its rating. It ends once a candidate rated at or above the list's lowest entry is examined. That candidate is dropped too, but it sets the recorded low to the list's lowest entry, so the next candidate competes normally.
- When several entries share the recorded low, a replacing candidate overwrites all of them. The list can then hold one object several times, and each copy counts toward the budget when orders are given.

The shortlist is sorted from highest to lowest rating, and defenders are ordered in that sequence. Each takes the [Rescue](/reference/enums/mission/) mission on a 66 percent chance and Area Guard otherwise. A member of an `IsBaseDefense=yes` team always takes Area Guard. Either way, the attacker becomes the defender's target, and the damaged object is recorded as what it is protecting.

After each order, the defender's `ThreatPosed` is added to a running total. Orders stop once the total exceeds the budget. The first defender is therefore always sent, even when it alone covers the whole budget.

A defender on Rescue first attacks the attacker. Once it has no target, it attacks [whatever it finds near the damaged object](/systems/target-selection/#when-an-object-scans). When nothing is left, it moves to a point its house chooses and switches to Area Guard on arrival:

- An object with no offensive value goes to the core of the base, a random point within one base radius of the base center.
- Every other object goes to one of the four edge zones at random. An edge zone is the ground in one compass quadrant of the base, between one and two base radii from the center.

For these points, the base radius counts as at least 3 and at most 8 cells.

### The attacker's cooldown

A call-up starts a cooldown of [`BaseDefenseDelay`](/keys/basedefensedelay/) minutes, set in `[General]`, when the `ThreatPosed` of the defenders it ordered exceeds the budget. The cooldown belongs to the attacker, not to the defending house. While it runs, no house calls defenders back against that attacker.

These call-ups set no cooldown and run again on the attacker's next hit:

- one that ran out of candidates before the defenders' total exceeded the budget;
- one whose budget reached exactly zero because of objects already targeting the attacker, as in [the strength budget](#the-strength-budget) example;
- one for an attacker whose `ThreatPosed` is `0`.

A budget pushed below zero by objects already targeting the attacker does start the cooldown.

## Campaign and skirmish differences

These rules differ between campaigns and skirmish or multiplayer games:

| Behavior | Campaign | Skirmish and multiplayer |
| --- | --- | --- |
| Picking a first enemy | Never happens | The nearest undefeated house whose country is not passive, once the countdown expires |
| A damaged object with a primary weapon | Calls no one back | Calls for help like any other |
| A candidate whose mission sets `Recruitable=no` | Rejected | Accepted; the mission's setting is not read |
| Which houses count as human | The house under player control | Every house a human plays |
| Breaking an alliance | Always proceeds | Requires that neither country sets `MultiplayPassive=yes` |

## Settings and state without effect

[`Whiner=yes`](/keys/whiner/) on a TeamType has no effect. A team member that takes damage always reports it to its team, which responds as [answering damage](/systems/ai-team-execution/#answering-damage) describes, so the setting never starts a call-up.

The [Autocreate Begins...](/mapping/actions/taction-autocreate/) trigger action has no effect on team creation. The team-creation step it was meant to start is disabled, and the step that replaced it does not read what the action sets. [`AutocreateTime`](/keys/autocreatetime/) belonged to [the same disabled step](/systems/ai-team-production/#parsed-settings-without-effect).
