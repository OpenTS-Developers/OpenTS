---
title: Target selection and threat rating
summary: "Scores the candidates an object could shoot at from per-type coefficients weighing armor, health, distance and the house's declared enemy."
category: combat-targeting
keys:
  - AA
  - AG
  - AV
  - ComputerBaseDefenseResponse
  - Elite
  - EnemyHouseThreatBonus
  - FireSupress
  - GuardArea
  - GuardRange
  - HasStupidGuardMode
  - IsThreatRatingNode
  - IsWebImmune
  - LegalTarget
  - MyEffectivenessCoefficient
  - MyEffectivenessCoefficientDefault
  - NoAutoFire
  - NoThreat
  - PlayerReturnFire
  - Primary
  - Retaliate
  - Secondary
  - SpecialThreatValue
  - Supress
  - TargetDistanceCoefficient
  - TargetDistanceCoefficientDefault
  - TargetEffectivenessCoefficient
  - TargetEffectivenessCoefficientDefault
  - TargetSpecialThreatCoefficient
  - TargetSpecialThreatCoefficientDefault
  - TargetStrengthCoefficient
  - TargetStrengthCoefficientDefault
  - ThreatPosed
  - Verses
  - WebDuration
  - Webby
related:
  - type: enum
    id: MissionType
  - type: enum
    id: QuarryType
  - type: system
    id: veterancy
---

## Missions in brief

This section introduces the entity the rest of the page turns on. Anyone already writing mission sections in a rules file can skip to [the five stages](#the-five-stages).

A **mission** is what an object is doing at this moment — guarding, moving toward a destination, hunting, harvesting, selling itself. Every object on the map is in exactly one, and the [mission](/reference/enums/mission/) page lists them all. A mission is not an order the player issues and not a line of a team's script: a player order, a script, a trigger action and the engine's own idle handling all work by putting an object into one, and nothing below depends on how the object got there.

Each mission also owns a rules section named after it — `[Guard]`, `[Area Guard]`, `[Hunt]`, `[Harmless]`, one per entry on that list. What is written there governs every object in that mission whatever type it is, which makes a mission section the one place a behavior can be changed for all of them at once. A mission whose section is absent keeps the engine's own values.

Four of the settings those sections carry reach the decisions on this page and on [engineers, capture and sabotage](/systems/capture/). The table names the decision each one lands in, so a reader tracing an object that will not shoot back, or will not step aside, knows which section to open.

| Setting | What it decides for an object in that mission |
| --- | --- |
| [`NoThreat`](/keys/nothreat/) | At `yes`, no other object's scan will [accept it as a candidate](#why-a-candidate-is-rejected). It is read from the candidate's mission, never from the scanning object's |
| [`Retaliate`](/keys/retaliate/) | At `no`, damage never makes it [turn on its attacker](#retaliation) |
| [`Scatter`](/keys/scatter/#scope-mission-behavior) | At `no`, the object will not scatter from a threat; on this page that is what stops an object [refused retaliation](#retaliation) from stepping aside |
| [`Rate`](/keys/rate/#scope-mission-behavior) | The fraction of a minute between one servicing pass of the mission and the next |

Written out, one mission section looks like this.

```ini title="rules.ini"
[Harmless]
NoThreat=yes
Retaliate=no
Rate=.5
```

An object put into Harmless is passed over by every scan, never fires back, and has its mission serviced about twice a minute.

## The five stages

An object scans for a target only when its current mission asks it to. Five stages then resolve one target, each narrowing what the next one sees. The table runs in that order: the first stage decides whether there is a scan at all, and only the last one scores anything.

| Stage | What it decides |
| --- | --- |
| Mission | Whether to scan, the coordinate to scan from, and which target categories are wanted |
| Object kind | Rewrites those categories from the object's own weapons and role |
| Scan | Walks rings of cells outward from the coordinate, or the whole object list |
| Cell | Offers at most one occupant per cell |
| Candidate | Rejects what may not be attacked, then scores what survives |

The survivor is assigned as the object's target. Retaliation is a separate path that never scans.

## When an object scans

### Mission entry points

These are the entry points that scan: the object missions that run one, plus the two team missions at the foot that make a member scan on the team's behalf. **A mission absent from this table never scans at all**, so an object left in one acquires a target only by being handed one or by retaliating. The middle column gives the shape and the reach of each scan — a *ring* scan walks whole cell rings outward from a coordinate and stops at the radius named, which [the next section](#scan-radius) derives, while a *whole map* scan walks the object lists instead and has no radius. The third column carries whatever else narrows the scan.

| Mission | Scan | Notes |
| --- | --- | --- |
| Guard | Ring, guard radius | Skipped on an engineer, and on a computer-controlled aircraft that already has a target. Other existing targets are validated and may be replaced |
| Guard area | Ring, area radius, from the home position | Runs only while the object has no target |
| Patrol | Ring, patrol radius, from the object, then re-scanned from the patrol cell | Also used by an aircraft that is patrolling with ammunition left |
| Move | Ring, guard radius | Only while the object has no target, only for a computer [house](/glossary/#house), and never for a member of a [`Suicide=yes`](/keys/suicide/) team |
| Hunt | Whole map | Outside campaigns an aircraft first asks for Tiberium processors alone — objects whose type declares a nonzero [`Storage`](/keys/storage/) — then repeats without restriction |
| Rescue | Whole map | The result is taken only when it is within 1.5 times the area radius of the spot the mission began at |
| Guard, on an armed building | Ring, guard radius | Not run on an EM pulse cannon, or on a building holding a chemical missile |
| Team attack | Whole map | The category comes from the team mission's [Quarry](/reference/enums/quarry/) |
| Team patrol | Ring, guard radius | Re-scanned from the team leader every [`PatrolScan`](/keys/patrolscan/) minutes |

On Guard, a non-engineer ground object and a human-controlled aircraft pass an existing target back through the range scan. A target that is no longer legal or within the guard radius is cleared before another candidate is considered, so the object does not keep chasing a stale target outside the area it guards. A computer-controlled aircraft keeps an existing target without this revalidation; with no target it runs the ring scan the table gives.

Three settings stop the scan before it starts. **Any of** them is enough, and each is read only on a human-owned object — a computer house ignores all three:

- [`NoAutoFire=yes`](/keys/noautofire/) on the object's type;
- the object is in Guard and either its type can cloak or it carries the `CLOAK` ability;
- [`DeployToFire=yes`](/keys/deploytofire/) on a vehicle.

### Scan radius

Three radii are derived from [`GuardRange`](/keys/guardrange/), the object type's own guard distance in [cells](/glossary/#cell). The table gives each one, and the mission table above says which of the three a mission asks for. The guard radius is the one with a fallback, and the paragraph after the table follows what that fallback changes.

| Radius | Value |
| --- | --- |
| Guard | `GuardRange`, when it is not zero and the object is not an engineer; otherwise weapon range |
| Area | Twice `GuardRange`, or twice the longer weapon range when `GuardRange` is zero, clamped to at most 16 cells |
| Patrol | The same figure, clamped to between 7 and 16 cells |

The scan walks that many whole cell rings. Where the guard radius falls back to weapon range, two things change at once: the ring count becomes the longer of the two weapon ranges in whole cells plus one, and each candidate has to pass a live range test as well. Which test that is depends on what the object carries.

- With a primary weapon, the candidate must be in range of the weapon chosen against it.
- With no primary weapon at all, there is no such test, and the candidate is accepted within `GuardRange`.

A healer — an object whose weapon damage averages below zero, such as a medic — overrides all of the above with a fixed 2 cells while it is in Guard.

:::caution[`GuardRange` is also a fence connection distance]
A [`LaserFencePost=yes`](/keys/laserfencepost/) building uses it as the number of cells it reaches along each of the four directions to find the next post, treating anything below one cell as one, and a [`FirestormWall=yes`](/keys/firestormwall/) type uses it as the number of cells a placed section searches for another section to join. Both truncate it to whole cells. The BuildingType IDs `GAFSDF`, `GAWALL` and `NAWALL` have it pinned to 5 cells immediately after their sections are read, so an assignment in those three sections is discarded.
:::

An unarmed building never scans at all — the scan lives on the armed branch of the Guard mission. [`HasStupidGuardMode=yes`](/keys/hasstupidguardmode/) additionally ends an unarmed building's Guard processing outright, so the repair-bay handover and the weapons-factory bib clearing stop running.

An idle object settles into either Guard or Area Guard, the mission that scans at the area radius, and a member of a team always takes plain Guard. What decides between them otherwise differs by kind.

- An infantry takes Area Guard while its house is computer-controlled and its [`IQ`](/keys/iq/) has reached the [`GuardArea`](/keys/guardarea/) level in `[IQ]`; at or above that level an armed infantry qualifies, and so does an unarmed engineer or vehicle thief, while below it every one of them takes Guard. A human-owned infantry takes Area Guard only while it carries the `GUARD_AREA` ability.
- An armed vehicle takes Area Guard once that same `IQ` level is reached, or whenever it carries the `GUARD_AREA` ability. Nothing on this path asks who owns the vehicle, so a human house whose scenario gives it the `IQ` sends its idle vehicles out as well.
- An armed aircraft takes Area Guard while it is computer-owned, whatever the level says.

## What each kind of object considers

The mission's request is rewritten before the scan runs.

For an infantry, a vehicle or a building, a request that names no category at all is filled in from the object's own weapons, one slot at a time. The table gives what each projectile setting contributes to that fill. The first row is the one to watch, because it ends the fill rather than adding to it.

| Projectile setting | Categories contributed |
| --- | --- |
| [`AV=yes`](/keys/av/) | Vehicles, and nothing else — the answer is returned immediately, so `AA` and `AG` on the same projectile contribute nothing |
| [`AA=yes`](/keys/aa/) | Aircraft |
| [`AG=yes`](/keys/ag/) | Infantry, vehicles and buildings |

An aircraft gets no such fill. It falls through to the rewrites below, so the categories its own projectile would have contributed never reach its own scans.

On top of that fill, each kind of object rewrites the request again. The table gives one row per kind, and the row that applies is the one for the object doing the looking rather than the one being looked at.

| Object | Rewrite |
| --- | --- |
| Anything that moves | An area request becomes a range request while the object is under a scan restriction — imposed when its path is completely blocked and its target is out of range, spread to every member when the object is in a team, and lifted as soon as a scan finds nothing. A still-empty request gains infantry, vehicles and buildings. |
| Infantry | See below. |
| Vehicle | A human-owned `DeployToFire=yes` vehicle finds nothing; otherwise only the weapon fill applies. |
| Building | Both weapon slots are added whether or not the request already named a category, a human-owned building drops buildings from the request, and the scan is forced into a ring. A building can never scan the whole map. |

Infantry make the most changes, in this order:

- A computer-owned unarmed [`Infiltrate=yes`](/keys/infiltrate/) infantry — a flag that [`C4=yes`](/keys/c4/) and [`Engineer=yes`](/keys/engineer/) also set — heads straight for the house's recapture target when it is within 15 cells, with no scan at all; otherwise capturable buildings are added to the request.
- A [`VehicleThief=yes`](/keys/vehiclethief/) infantry already driving toward a non-[`IsTrain`](/keys/istrain/) vehicle within 15 cells keeps it, again without scanning. These two are the only paths that reach a target without scoring it.
- An unarmed infantry that is neither of those finds nothing. An unarmed vehicle thief has buildings and aircraft struck out of its request and vehicles put in.
- When the primary weapon's warhead is organic, buildings, vehicles and aircraft are struck out, leaving infantry. Organic is derived, not configured: a warhead counts as organic when its [`Verses`](/keys/verses/) percentage against `heavy` armor is exactly `0%`. This is what confines a dog or a medic to infantry.
- A human-owned armed infantry drops buildings; a computer-owned `C4=yes` infantry, or one carrying the `C4` ability, puts them back.
- A [`Thief=yes`](/keys/thief/) infantry adds capturable buildings and Tiberium processors.

Two more rewrites apply to any object. A healer has its whole request replaced by infantry plus allies if it is an infantry, or vehicles plus allies if it is a vehicle. An engineer, whoever owns it, has infantry and vehicles struck out.

## Why a candidate is rejected

Every candidate the scan offers runs this gauntlet in order, and the first row that matches ends it — nothing further down is consulted and no score is computed. The left column is the test and the right is what it turns on, so a row with an empty right column has nothing else to it.

| Rejected when | Detail |
| --- | --- |
| The candidate is in [limbo](/glossary/#limbo), or already at zero strength | |
| It is cloaked, its cell is not sensed by this house, and it belongs to another house | |
| It has not yet entered the playable area | Reinforcements still crossing in from off map are not targeted |
| Its current mission sets [`NoThreat=yes`](/keys/nothreat/) | |
| It sits more than 20 leptons below ground level | A lepton is the engine's internal distance unit; 256 of them make one cell |
| It is in a different [movement zone](/glossary/#movement-zone) | The cell is tested on a ring scan and the candidate on a full-map scan; neither test runs for a range-limited request, nor for a building or an aircraft |
| It is an ally | Unless this object heals or is an engineer and the ally is below full strength; a berzerk infantry — a [`Cyborg=yes`](/keys/cyborg/) type sent out of its mind by damage under [`BerzerkAllowed=yes`](/keys/berzerkallowed/), after which it attacks whatever is near it — ignores allegiance entirely. A healing vehicle additionally rejects an airborne ally, an ally standing in a building's cell, and any ally that is not a vehicle |
| Harvester immunity is on and its type is listed in [`HarvesterUnit`](/keys/harvesterunit/) | From the multiplayer harvester truce, or from a scenario's [`HarvesterImmune`](/keys/harvesterimmune/) setting |
| It is beyond the scan radius | Or, when the scan uses weapon range, out of range of the weapon chosen against it |
| Campaign games only, and only when the scanning house is under player control: the candidate is not the player's, has never been discovered by the player, and is not an aircraft | A computer house's campaign scan never applies this filter |
| It is a building whose type sets [`InvisibleInGame=yes`](/keys/invisibleingame/) | |
| Its kind is not among the requested categories | A landed aircraft counts as a vehicle, and so does a building that can undeploy into one unless it is a construction yard |
| Skirmish and multiplayer only: its house's country sets [`MultiplayPassive=yes`](/keys/multiplaypassive/) | |
| Its type sets [`LegalTarget=no`](/keys/legaltarget/) | |
| It is an `IsTrain=yes` type and this object is a vehicle thief | |
| It is a [`Disguised=yes`](/keys/disguised/) infantry | Unless this object's type sets [`DetectDisguise=yes`](/keys/detectdisguise/), or [`AIDetectDisguise=yes`](/keys/aidetectdisguise/) is set and the scanning house is not under player control |
| This object's primary projectile is `AG=no` and the candidate is at ground level | This rejects every ground object, not only landed aircraft, and it reads the primary slot even when the secondary would be fired |
| The request asked for civilians | |
| The request asked for capturable buildings and this is not a [`Capturable=yes`](/keys/capturable/) building | |
| A human-owned object outside a team is looking at an unarmed building | Unless the object is an engineer, or the building can undeploy into a vehicle |
| This object is an engineer and the candidate is not a building, or is an ally above [`ConditionRed`](/keys/conditionred/), or is an ally that costs nothing | |
| The request asked for Tiberium processors and the candidate's [`Storage`](/keys/storage/) is zero | |
| Both stand in bridge cells and only one of the two is up on the bridge | |
| A web warhead is in play and the candidate is an infantry that has been struggling for longer than a quarter of [`WebDuration`](/keys/webduration/) | The second slot's warhead is consulted when it is [`Webby=yes`](/keys/webby/), otherwise the first slot's |

## The threat score

Whatever survives is scored. Five coefficients drive the score, and the table gives what each one multiplies and which way a positive value pushes the choice. All five are read from the type of the object doing the choosing, never from the candidate's type, so retuning one of them changes what that one type prefers and nothing about how it is preferred by others.

| Setting | Multiplies | Sign |
| --- | --- | --- |
| [`TargetEffectivenessCoefficient`](/keys/targeteffectivenesscoefficient/) | The `Verses` percentage of the candidate's chosen warhead against this object's armor | Positive prefers candidates that can hurt this object |
| [`TargetSpecialThreatCoefficient`](/keys/targetspecialthreatcoefficient/) | The candidate type's [`SpecialThreatValue`](/keys/specialthreatvalue/) | Positive prefers a high `SpecialThreatValue` |
| [`MyEffectivenessCoefficient`](/keys/myeffectivenesscoefficient/) | The `Verses` percentage of this object's chosen warhead against the candidate's armor | Positive prefers candidates this object can hurt |
| [`TargetStrengthCoefficient`](/keys/targetstrengthcoefficient/) | The candidate's current strength as a fraction of its maximum, from `0` to `1` | Positive prefers healthy candidates, negative prefers wounded ones |
| [`TargetDistanceCoefficient`](/keys/targetdistancecoefficient/) | How far beyond weapon range the candidate lies, and zero inside it | Must be negative to penalize distance; positive rewards distant candidates |

[`Verses`](/keys/verses/) is the multiplier in both effectiveness terms, read as a fraction against the candidate's or this object's [armor class](/reference/enums/armor/) — `none`, `wood`, `light`, `heavy` or `concrete`.

```text title="threat score, in evaluation order"
threat = ± TargetEffectivenessCoefficient × Verses of the candidate's warhead vs. my armor
       +   TargetSpecialThreatCoefficient × the candidate's SpecialThreatValue
       +   EnemyHouseThreatBonus                        (candidate belongs to the declared enemy)
       +   MyEffectivenessCoefficient     × Verses of my warhead vs. the candidate's armor
       +   TargetStrengthCoefficient      × the candidate's health fraction
       +   TargetDistanceCoefficient      × max(0, distance − weapon range)
       +   100000
```

The `100000` added to every score is fixed in the engine. [`EnemyHouseThreatBonus`](/keys/enemyhousethreatbonus/) is a flat addition, applied when the candidate's house is the one this house has [declared as its enemy](/systems/base-attacked/#anger-and-the-declared-enemy). The finished figure is stored as a whole number, so the fraction is discarded.

:::caution[The stock coefficients are all zero]
Every one of the five, and `EnemyHouseThreatBonus`, starts at zero. With a rules file that sets none of them every surviving candidate scores exactly `100000`, and the choice falls through to the order the scan happened to visit them in. Threat rating does nothing at all until at least one coefficient is written.
:::

:::caution[A candidate already shooting at this object scores lower]
The first term's sign is flipped when the candidate's own target is this object, so a candidate shooting at something else outscores an otherwise identical one shooting at this object by twice that term. A positive `TargetEffectivenessCoefficient`, meant to draw fire toward dangerous targets, therefore draws it toward the dangerous targets that are shooting at somebody else.
:::

:::caution[The distance term acts only outside weapon range]
`max(0, distance − range)` is zero for anything the object can already shoot, so `TargetDistanceCoefficient` changes nothing among in-range candidates. Range is always in cells; distance is in cells too, except on the full-map ground scan, which measures it in leptons instead, so the same coefficient bites roughly 256 times harder there than on a ring scan.
:::

### Where the coefficients come from

Each per-type coefficient is read with a fallback: when the type's stored value is zero, the matching `[General]` setting — [`MyEffectivenessCoefficientDefault`](/keys/myeffectivenesscoefficientdefault/), [`TargetEffectivenessCoefficientDefault`](/keys/targeteffectivenesscoefficientdefault/), [`TargetSpecialThreatCoefficientDefault`](/keys/targetspecialthreatcoefficientdefault/), [`TargetStrengthCoefficientDefault`](/keys/targetstrengthcoefficientdefault/) and [`TargetDistanceCoefficientDefault`](/keys/targetdistancecoefficientdefault/) — supplies the value instead. `SpecialThreatValue` has no such fallback.

:::caution[A per-type coefficient cannot be pinned at zero]
The rules are read again for each later layer — the language rules, the expansion rules, and the map. Each layer that carries the type's section re-runs that same fallback, so a coefficient explicitly written as `0` is replaced by the global default on the first later layer whose copy of the section omits the key. Zero survives only while the matching `[General]` default is itself zero.
:::

```ini title="rules.ini"
[General]
TargetEffectivenessCoefficientDefault=1
TargetDistanceCoefficientDefault=-1

[MYTANK] ; example UnitType
MyEffectivenessCoefficient=2
TargetStrengthCoefficient=-0.5
```

`MYTANK` weighs what it can hurt at twice the rate, prefers wounded candidates, and takes the two global defaults for the two coefficients it does not name.

:::danger[The per-type coefficients are always the ones used]
A second, house-wide set of coefficients exists behind a flag that is meant to be switched on by owning a threat rating structure. Every house sets that flag the moment it is created, and nothing ever clears it, so the per-type coefficients above are in force from the first frame of every game and the house-wide set is unreachable. [`IsThreatRatingNode=yes`](/keys/isthreatratingnode/) still has two consumers, both on the upgrade path: plugging such an upgrade into a host building sets the flag, and so does removing a host that carries one. A standalone building with the setting, one that is not a [`PowersUpBuilding`](/keys/powersupbuilding/) upgrade, triggers neither.
:::

### Adjustments after scoring

- A house ordered to hunt everything clamps the score of any candidate that does not belong to its declared enemy down to exactly `1`.
- A request for power plants adds `Power × 1000` for a building with a positive [`Power`](/keys/power/) and zeroes anything else.
- A request for factories zeroes a building that produces nothing.
- A request for base defenses zeroes any candidate with no primary weapon.
- When this object's primary weapon sets [`Supress=yes`](/keys/supress/), the score is halved once for every ring cell holding an allied building around the candidate's cell, out to the [`FireSupress`](/keys/firesupress/) distance in `[CombatDamage]` — a building spanning several ring cells halves it several times. The default of `1` cell leaves no ring to walk, so the check never fires until `FireSupress` is raised.
- A score of exactly zero rejects the candidate; anything else is raised to at least `1`.

## Which weapon the score assumes

Both effectiveness terms, and every range test in the gauntlet, use the weapon each side would choose against the other. Each of the two slots is scored as its warhead's `Verses` percentage against the target's armor multiplied by `1000`, doubled when the target is within that slot's range, and zeroed when the slot cannot fire at that moment — no line of fire, an illegal target, or still reloading. The higher score wins and a tie takes the primary. A target that is not an object at all is treated as armor class `none`.

The primary slot is not always [`Primary`](/keys/primary/). An elite object substitutes its [`Elite`](/keys/elite/) weapon into that slot, and a building resolves the slot through its plugged-in upgrades before either; [the elite weapon](/systems/veterancy/#the-elite-weapon) covers both substitutions. Everything on this page that reads "primary weapon" means the weapon that slot resolves to.

:::danger[A web primary with no secondary reads an empty weapon slot]
When the primary's warhead is `Webby=yes` and the secondary slot is empty, weapon choice returns the secondary slot for every target the web weapon could otherwise fire at but cannot web — every vehicle and building, a landed aircraft, and every [`IsWebImmune=yes`](/keys/iswebimmune/) or immobilized infantry. Scoring tolerates the empty slot and falls back to `GuardRange` for the range term, but the retaliation check reads that slot's warhead without testing whether the slot is filled, so such an object fails as soon as it is damaged by anything it cannot web. Give any object with a web primary a secondary weapon.
:::

## Picking the winner

:::danger[A ring scan does not rank its candidates]
The four loops that walk a ring of cells never raise the running best score, so each qualifying candidate simply replaces the one before it and the object returned is the last one visited, not the highest-scoring. Every guard, guard area, patrol, move and building scan is a ring scan. Tuning the coefficients still decides whether a candidate qualifies at all, but among those that qualify it changes nothing; only the full-map scan compares scores.
:::

When aircraft are wanted, the two passes over the flying layers run before the rings and do keep the highest score. Those passes consider aircraft and [`JumpJet=yes`](/keys/jumpjet/) infantry only — a vehicle flying on a jumpjet locomotor matches neither pass. The ring loops that follow compare against whatever figure those passes left behind, but never update it themselves.

A ring scan stops early: once it holds any object it returns at the ring that is a quarter of the scan radius and again at the ring that is half of it. A landed aircraft is found by the ground rings whenever vehicles are wanted.

While no object has been found, each ring also weighs its cells as wall targets. Wall targeting is computer-only: a human-owned object weighs no cell at all. Beyond that, a cell is weighed only while **none of** these holds, and the first one that does ends the weighing there:

1. the difficulty sets [`DestroyWalls=no`](/keys/destroywalls/);
2. the cell carries no overlay, or carries one whose type is not [`Wall=yes`](/keys/wall/);
3. the wall is out of range of the weapon chosen against it;
4. this object has no primary weapon, or its primary weapon names no warhead;
5. the primary weapon's projectile sets `AG=no`;
6. the primary weapon's warhead is not `Wall=yes`;
7. the wall is unowned, or belongs to a house this one is allied with.

Only the third test reads the weapon chosen against the wall. The three after it read the primary slot whether or not that is the slot that would fire, so a secondary weapon built to break walls never makes a wall a target. A wall that passes all seven is scored by how far inside the primary weapon's range it sits, and ends the scan at the end of the ring that found it.

The full-map scan is the only path that ranks: it walks the aircraft list and then every ground object, keeping the highest score at each step.

## Retaliation

Damage is answered without any scan. The table runs top to bottom and the first row an object matches settles the question, so the reason a particular object did not fire back is the first row it matches and nothing below it.

| Condition | Retaliates |
| --- | --- |
| Human-owned and already has a target | No — player orders are not overridden |
| The warhead is [`Veinhole=yes`](/keys/veinhole/) | Yes, unless the object is human-owned and moving under orders |
| No source object | No |
| The current mission sets [`Retaliate=no`](/keys/retaliate/) | No |
| The source is an ally | No |
| The object's weapons average zero damage or less, or it has no primary weapon | No |
| The chosen weapon's `Verses` against the source's armor is `0%` | No |
| The source is an aircraft and the chosen weapon's projectile is `AA=no` | No |
| Human-owned `C4=yes` infantry, or any human-owned object carrying the `C4` ability, damaged by a building | No |
| Human-owned vehicle whose [`DeploysInto`](/keys/deploysinto/) type is [`Artillary=yes`](/keys/artillary/) | No |
| Human-owned non-building, with [`PlayerReturnFire=no`](/keys/playerreturnfire/) in `[CombatDamage]`, outside Guard, Area Guard and Patrol | No |
| Member of a `Suicide=yes` team | No |
| Computer-owned, already shooting at something that scores higher than the source | No |
| Otherwise | Yes |

That last row is the only place a threat score is compared at full precision rather than being truncated to a whole number.

An object that does retaliate attacks the source directly, overriding its current mission. It does so at once when the source is in range; out of range, a computer house retaliates from any distance and a human house only when the source is within [`Sight`](/keys/sight/) plus half a cell.

A damaged building has a second, scan-free path besides this one: left idle with no target, it acquires its attacker directly — for a human house only with [`PlayerReturnFire=yes`](/keys/playerreturnfire/) or against an aircraft.

Scattering fills the gap on either branch, and only for an object that moves. The two branches ask for different things.

An object the table allowed to retaliate, but that came out of it holding neither a target nor a destination, scatters under **any of**:

- [`PlayerScatter=yes`](/keys/playerscatter/) in `[CombatDamage]`;
- the `SCATTER` ability.

Turning on the attacker sets the target in the same step, so this branch is reached only where the permitted retaliation produced no attack at all: a `Veinhole=yes` warhead with no monster behind it to turn on, or a human-owned object whose attacker lies beyond both weapon range and `Sight` plus half a cell.

An object refused retaliation scatters under **all of**:

- its current mission sets [`Scatter=yes`](/keys/scatter/#scope-mission-behavior);
- it is untethered;
- it is stationary;
- it holds neither a target nor a destination;
- it is not an aircraft;
- **any of:** the object is computer-owned; `PlayerScatter=yes` is set; it carries the `SCATTER` ability.

## Threat ratings that are not target selection

Two settings named for threat play no part in any of the above.

[`ThreatPosed`](/keys/threatposed/) is what an object is worth as a danger, not what it is worth as a target. It feeds the value an object carries when a team weighs its members, the per-region threat map each house keeps, and how large a computer house's [base defense response](/systems/base-attacked/#the-strength-budget) is, which comes from the attacker's figure and [`ComputerBaseDefenseResponse`](/keys/computerbasedefenseresponse/) in `[AI]`.

[`ThreatAvoidanceCoefficient`](/keys/threatavoidancecoefficient/) is read only by the pathfinder, where it scales the threat map figure a route is allowed to cross. A team's [`AvoidThreats=yes`](/keys/avoidthreats/) raises it to `1` for its members.

## Campaign and skirmish differences

- The undiscovered-target rejection applies in campaigns only.
- The `MultiplayPassive` rejection, and an aircraft's first pass for Tiberium processors on Hunt, apply outside campaigns only.
- Harvester immunity comes from the multiplayer lobby's harvester truce outside campaigns and from a scenario's `HarvesterImmune` setting inside them.
- Outside campaigns the warhead named `ARTYHE` has its entire `Verses` table replaced by fixed values, which changes every effectiveness term computed from it.

## Settings without effect

The five house-wide coefficients — [`DumbMyEffectivenessCoefficient`](/keys/dumbmyeffectivenesscoefficient/), [`DumbTargetEffectivenessCoefficient`](/keys/dumbtargeteffectivenesscoefficient/), [`DumbTargetSpecialThreatCoefficient`](/keys/dumbtargetspecialthreatcoefficient/), [`DumbTargetStrengthCoefficient`](/keys/dumbtargetstrengthcoefficient/) and [`DumbTargetDistanceCoefficient`](/keys/dumbtargetdistancecoefficient/) — are parsed and stored but can never be reached, because the flag that would select them is already set on every house.

Two scan categories are inert as well. Nothing ever asks for civilians, and a request that did would reject every candidate it examined. The boats category is contributed by `AG=yes` but is never turned into a target kind, so it neither widens nor narrows a scan.
