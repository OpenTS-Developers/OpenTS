---
title: Target selection and threat rating
summary: "How objects pick targets on their own, which candidates they reject, and how per-type threat coefficients score the rest."
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
  - Mechanic
  - MyEffectivenessCoefficient
  - MyEffectivenessCoefficientDefault
  - NoAutoFire
  - NoThreat
  - OmniHealer
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

Anyone already writing mission sections in a rules file can skip to [the five stages](#the-five-stages).

A **mission** is what an object is doing at this moment: guarding, moving toward a destination, hunting, harvesting, selling itself. Every object on the map is in exactly one, and the [mission](/reference/enums/mission/) page lists them all. A player order, a team script, a trigger action and the engine's idle handling all work by putting an object into a mission. Nothing below depends on how the object got there.

Each mission has a rules section named after it, such as `[Guard]`, `[Area Guard]`, `[Hunt]` or `[Harmless]`. Its settings apply to every object in that mission, whatever the object's type, so a mission section changes a behavior for all of them at once. A mission with no section keeps the engine's values.

Four mission settings affect the decisions on this page and on [engineers, capture and sabotage](/systems/capture/).

| Setting | What it decides for an object in that mission |
| --- | --- |
| [`NoThreat`](/keys/nothreat/) | At `yes`, no other object's scan will [accept it as a candidate](#why-a-candidate-is-rejected). The candidate's mission decides this, not the scanning object's |
| [`Retaliate`](/keys/retaliate/) | At `no`, damage never makes it [turn on its attacker](#retaliation) |
| [`Scatter`](/keys/scatter/#scope-mission-behavior) | At `no`, the object does not scatter from a threat. On this page, that stops an object [refused retaliation](#retaliation) from stepping aside |
| [`Rate`](/keys/rate/#scope-mission-behavior) | The fraction of a minute between one servicing pass of the mission and the next |

Written out, one mission section looks like this.

```ini title="rules.ini"
[Harmless]
NoThreat=yes
Retaliate=no
Rate=.5
```

An object in Harmless is passed over by every scan, does not turn on an attacker, and has its mission serviced about twice a minute. It stays harmless only until an attacker damages it. An object outside a team then returns to its idle mission unless the section also sets [`Zombie=yes`](/keys/zombie/).

## The five stages

An object scans for a target only when its current mission calls for a scan. Five stages then produce one target, and each stage narrows what the next one sees. The first stage decides whether there is a scan at all, and only the last one scores anything.

| Stage | What it decides |
| --- | --- |
| Mission | Whether to scan, the point to scan from, and which kinds of target are wanted |
| Object kind | Rewrites those kinds from the object's weapons and role |
| Scan | Walks rings of cells outward from the scan point, or walks every object on the map |
| Cell | Offers at most one occupant per cell |
| Candidate | Rejects what may not be attacked, then scores what is left |

The object takes the chosen candidate as its target. Retaliation is a separate path that never scans.

## When an object scans

### Mission entry points

The table lists every mission that scans. The last two rows are team missions, in which one member scans on the team's behalf. A mission not in this table **never scans**. An object in one gets a target only by being given one or by retaliating.

The middle column gives each scan's shape and reach. A *ring* scan walks whole rings of cells outward from a point and stops at the radius named; [the next section](#scan-radius) gives the three radii. A *whole map* scan walks every object instead and has no radius.

| Mission | Scan | Notes |
| --- | --- | --- |
| Guard | Ring, guard radius | Not run on an engineer. An aircraft seldom reaches this scan. It attacks a target it already holds, returns to idle handling at cruising height, and does not scan while landed unless it is docked. A human-owned aircraft scans only while airborne. A computer-owned aircraft whose house is not under attack first looks for an enemy vehicle outside that vehicle's base, preferring near ones and ones that cannot fire at aircraft. If it finds one, it attacks it without scanning |
| Guard area | Ring, area radius, from the home position | Runs only while the object has no target |
| Patrol | Ring, patrol radius, from the object, then re-scanned from the patrol cell | An aircraft on patrol with ammunition left runs only the first scan |
| Move | Ring, guard radius | Runs only while the object has no target, only for a computer [house](/glossary/#house), never on an aircraft, and never for a member of a [`Suicide=yes`](/keys/suicide/) team |
| Hunt | Whole map | Outside campaigns, an aircraft first scans for Tiberium processors alone (objects whose type sets a nonzero [`Storage`](/keys/storage/)), then scans again without that restriction |
| Rescue | Whole map | The result is kept only when it lies within 1.5 times the area radius of the spot where the mission began |
| Guard, on an armed building | Ring, guard radius | Runs only while the building has no target. Not run on an EM pulse cannon, or on a building that holds a chemical missile |
| Team attack | Whole map | The team mission's [Quarry](/reference/enums/quarry/) sets the kind of target. A TeamType with [`OnlyTargetHouseEnemy=yes`](/keys/onlytargethouseenemy/) considers only its house's declared enemy |
| Team patrol | Ring, guard radius | The team leader scans every [`PatrolScan`](/keys/patrolscan/) minutes |

In Guard, an infantry or vehicle that is not an engineer checks its current target before scanning. If the target is out of range of the weapon chosen against it, the object drops it and scans for another. It therefore does not keep chasing something it can no longer shoot.

Three conditions stop a scan before it starts. **Any of** them is enough, and each applies only to a human-owned object:

- [`NoAutoFire=yes`](/keys/noautofire/) on the object's type;
- the object is in Guard, and it can cloak (from its type or a cloaking crate) or holds the `CLOAK` ability;
- [`DeployToFire=yes`](/keys/deploytofire/) on a vehicle.

### Scan radius

Three scan radii are derived from [`GuardRange`](/keys/guardrange/), the object type's guard distance in [cells](/glossary/#cell). The mission table above says which one a mission uses.

| Radius | Value |
| --- | --- |
| Guard | `GuardRange`, when it is not zero and the object is not an engineer. Otherwise the object's weapon range, as described below |
| Area | Twice `GuardRange`, or twice the longer weapon range when `GuardRange` is zero. At most 16 cells |
| Patrol | The same figure as the area radius, but kept between 7 and 16 cells |

The scan walks that many whole rings of cells.

When the guard radius falls back to weapon range, the scan changes in two ways:

- It walks as many rings as the longer of the two weapon ranges, in whole cells, plus one. An object with no weapon at all therefore scans only the cell it stands in.
- Each candidate must also pass a range test. An object with a primary weapon accepts a candidate only within range of the weapon it would choose against it. An object with no primary weapon accepts a candidate within `GuardRange`.

A healer is an object whose weapons average negative damage, such as a medic. In Guard, a healer scans a fixed 2 cells, whatever the radius rules above give.

:::caution[`GuardRange` is also a fence connection distance]
A [`LaserFencePost=yes`](/keys/laserfencepost/) building uses `GuardRange` as the number of cells it searches in each of the four cardinal directions for the next post, and treats anything below one cell as one. A [`FirestormWall=yes`](/keys/firestormwall/) type uses it as the number of cells a placed section searches for another section to join. Both round it down to whole cells, so changing `GuardRange` on those types changes how far a fence run reaches.
:::

An unarmed building never scans. [`HasStupidGuardMode=yes`](/keys/hasstupidguardmode/) goes further: it ends an unarmed building's Guard processing outright. A repair bay with this setting no longer hands off a waiting unit, and a weapons factory no longer clears its exit.

An idle object settles into either Guard or Area Guard, and Area Guard is the mission that scans at the area radius. What sends an idle object to Area Guard depends on its kind:

- A computer-owned infantry outside a team takes Area Guard once its house's [`IQ`](/keys/iq/) reaches the [`GuardArea`](/keys/guardarea/) level in `[IQ]`, provided it is armed, an engineer or a vehicle thief. Other unarmed infantry take Guard, and so does every infantry below that level.
- A human-owned infantry takes Area Guard only through the `GUARD_AREA` ability, and only while it is outside a team.
- An armed vehicle outside a team takes Area Guard once its house reaches that `IQ` level, or whenever it holds the `GUARD_AREA` ability. The `IQ` test does not check who owns the vehicle, so a human house whose scenario gives it that `IQ` sends its idle vehicles to Area Guard as well.
- An armed aircraft takes Area Guard while it is computer-owned and outside a team, whatever its house's `IQ`.

## What each kind of object considers

Before the scan runs, the kind of object doing the looking rewrites the list of wanted target kinds.

When the mission asks for no particular kind, an infantry or a vehicle fills the list from its weapons, one weapon slot at a time.

| Projectile setting | Target kinds added |
| --- | --- |
| [`AV=yes`](/keys/av/) | Vehicles only. `AA` and `AG` on the same projectile add nothing |
| [`AA=yes`](/keys/aa/) | Aircraft |
| [`AG=yes`](/keys/ag/) | Infantry, vehicles and buildings |

An aircraft does not fill the list from its weapons. It takes the ground fill in the next table instead, so the kinds its projectile would add never reach its scans.

Each kind of object then changes the list further. The row that applies is the one for the object doing the looking, not the one being looked at.

| Object | Changes |
| --- | --- |
| Anything that moves | A list that names no infantry, vehicle or building kind gains all three |
| Anything that moves, under a scan restriction | An area scan becomes a range scan. The restriction starts when the object stops because its path is completely blocked while its target is out of range. In a team, every member gets it. It lifts as soon as a scan finds nothing |
| Infantry | See below |
| Vehicle | A human-owned `DeployToFire=yes` vehicle finds nothing. Otherwise only the weapon fill applies |
| Building | Adds what both weapon slots can attack, even when the mission already named a kind. A human-owned building drops buildings from the list. A building always runs a ring scan, never a whole-map scan |

Infantry make the most changes. They apply in this order:

- A computer-owned unarmed [`Infiltrate=yes`](/keys/infiltrate/) infantry goes straight for its house's recapture target when that building is within 15 cells, without scanning. Otherwise its scan is limited to capturable buildings. [`C4=yes`](/keys/c4/) and [`Engineer=yes`](/keys/engineer/) both imply `Infiltrate=yes`.
- A [`VehicleThief=yes`](/keys/vehiclethief/) infantry already heading for a vehicle within 15 cells that is not [`IsTrain=yes`](/keys/istrain/) keeps that vehicle as its target, without scanning.
- An unarmed infantry that is neither `Infiltrate=yes` nor `VehicleThief=yes` finds nothing. An unarmed vehicle thief looks for vehicles, never buildings or aircraft.
- An infantry whose primary weapon has an organic warhead considers only infantry. A warhead counts as organic when its [`Verses`](/keys/verses/) percentage against `heavy` armor is exactly `0%`; no setting marks it directly. This is what confines a dog to infantry. A healer's list is replaced by the healer rule below, so this rule never decides what a healer treats.
- A human-owned armed infantry drops buildings. A computer-owned `C4=yes` infantry, or one holding the `C4` ability, adds them.
- A [`Thief=yes`](/keys/thief/) infantry considers only capturable buildings that have a nonzero `Storage`.

The organic rule and the human-owned rule do not undo a team attack's [Quarry](/reference/enums/quarry/). A quarry that names a kind of building, such as power plants or factories, still reaches buildings.

Two more rules apply after these, to infantry and vehicles:

- A healing infantry or vehicle considers only damaged allies of the kinds it mends. An infantry mends infantry and a vehicle mends vehicles; [`Mechanic=yes`](/keys/mechanic/) switches that to vehicles and [`OmniHealer=yes`](/keys/omnihealer/) to both.
- An engineer, whoever owns it, never considers infantry or vehicles.

## Why a candidate is rejected

The engine tests each candidate against these rows in order and rejects it at the first row that matches. A rejected candidate is not scored.

| Rejected when | Detail |
| --- | --- |
| The candidate is in [limbo](/glossary/#limbo), or already at zero strength | |
| It is cloaked, its cell is not sensed by this house, and it belongs to another house | |
| It has not yet entered the playable area | Reinforcements still crossing in from off the map are not targeted |
| Its current mission sets [`NoThreat=yes`](/keys/nothreat/) | |
| It is more than 20 leptons below ground level | A cell is 256 leptons across |
| It is in a different [movement zone](/glossary/#movement-zone) | A ring scan tests the cell and a whole-map scan tests the candidate. Neither test runs for a range scan, or when the scanning object is a building or an aircraft |
| It is an ally | Unless this object heals or is an engineer and the ally is damaged. A healer also rejects an allied aircraft that is airborne or standing in a building's cell, and an ally of a kind it does not mend. A berzerk infantry skips this row and attacks whatever is near it. Infantry goes berzerk through the [Go Berzerk](/mapping/actions/taction-go-berzerk/) trigger action or [team mission](/mapping/missions/tmission-berzerk/), or from damage when its type is [`Cyborg=yes`](/keys/cyborg/) under [`BerzerkAllowed=yes`](/keys/berzerkallowed/) |
| Harvester immunity is on and its type is listed in [`HarvesterUnit`](/keys/harvesterunit/) | Immunity comes from the multiplayer harvester truce or from a scenario's [`HarvesterImmune`](/keys/harvesterimmune/) setting |
| It is beyond the scan radius | Or, when the scan uses weapon range, out of range of the weapon chosen against it |
| Campaigns only, when the scanning house is the player's: the candidate is not the player's, the player has never discovered it, and it is not an aircraft | A computer house's campaign scan skips this row |
| It is a building whose type sets [`InvisibleInGame=yes`](/keys/invisibleingame/) | |
| Its kind is not among the wanted kinds | A landed aircraft counts as a vehicle. So does a building that can undeploy into a vehicle, unless it is a construction yard |
| Skirmish and multiplayer only: its house's country sets [`MultiplayPassive=yes`](/keys/multiplaypassive/) | Unless the match lets objects attack neutral houses |
| Its type sets [`LegalTarget=no`](/keys/legaltarget/) | |
| It is an `IsTrain=yes` type and this object is a vehicle thief | |
| It is a [`Disguised=yes`](/keys/disguised/) infantry | Unless this object's type sets [`DetectDisguise=yes`](/keys/detectdisguise/), or [`AIDetectDisguise=yes`](/keys/aidetectdisguise/) is set and the scanning house is computer-controlled |
| This object's primary projectile is `AG=no` and the candidate is at ground level | This rejects every ground object, not only landed aircraft. It reads the primary slot even when the secondary would be fired |
| The scan asked for civilians | |
| The scan is limited to capturable buildings and this is not a [`Capturable=yes`](/keys/capturable/) building | |
| A human-owned object outside a team is looking at a building that cannot shoot back | That is a building with no weapon in its first slot, or one whose weapon has no range. Engineers are exempt, and so is a building that can undeploy into a vehicle |
| This object is an engineer and the candidate is not a building, or is an ally above [`ConditionRed`](/keys/conditionred/), or is an ally that costs nothing | |
| The scan is limited to Tiberium processors and the candidate's [`Storage`](/keys/storage/) is zero | |
| Both stand in bridge cells, and only one of the two is up on the bridge | |
| This object has a web warhead and the candidate is an infantry still webbed for more than a quarter of the warhead's [`WebDuration`](/keys/webduration/) | The secondary slot's warhead is used when it is [`Webby=yes`](/keys/webby/), otherwise the primary slot's |

## The threat score

Each candidate that passes gets a threat score from five coefficients. All five come from the type of the object doing the choosing, never from the candidate's type. Changing one changes what that type prefers, not how other types treat it.

| Setting | Multiplies | Sign |
| --- | --- | --- |
| [`TargetEffectivenessCoefficient`](/keys/targeteffectivenesscoefficient/) | The `Verses` percentage of the candidate's chosen warhead against this object's armor | Positive prefers candidates that can hurt this object |
| [`TargetSpecialThreatCoefficient`](/keys/targetspecialthreatcoefficient/) | The candidate type's [`SpecialThreatValue`](/keys/specialthreatvalue/) | Positive prefers a high `SpecialThreatValue` |
| [`MyEffectivenessCoefficient`](/keys/myeffectivenesscoefficient/) | The `Verses` percentage of this object's chosen warhead against the candidate's armor | Positive prefers candidates this object can hurt |
| [`TargetStrengthCoefficient`](/keys/targetstrengthcoefficient/) | The candidate's current strength as a fraction of its maximum, from `0` to `1` | Positive prefers healthy candidates, negative prefers wounded ones |
| [`TargetDistanceCoefficient`](/keys/targetdistancecoefficient/) | How far beyond weapon range the candidate lies, and zero inside it | Negative penalizes distance; positive rewards distant candidates |

Both effectiveness terms use [`Verses`](/keys/verses/) as a fraction against the relevant [armor class](/reference/enums/armor/): `none`, `wood`, `light`, `heavy` or `concrete`.

```text title="threat score, in evaluation order"
threat = ± TargetEffectivenessCoefficient × Verses of the candidate's warhead vs. my armor
       +   TargetSpecialThreatCoefficient × the candidate's SpecialThreatValue
       +   EnemyHouseThreatBonus                        (candidate belongs to the declared enemy)
       +   MyEffectivenessCoefficient     × Verses of my warhead vs. the candidate's armor
       +   TargetStrengthCoefficient      × the candidate's health fraction
       +   TargetDistanceCoefficient      × max(0, distance − weapon range)
       +   100000
```

Every score includes a fixed `100000`. [`EnemyHouseThreatBonus`](/keys/enemyhousethreatbonus/) is a flat addition for a candidate that belongs to the house this house has [declared as its enemy](/systems/base-attacked/#anger-and-the-declared-enemy). The finished score is rounded down to a whole number.

:::caution[Unset coefficients are zero]
All five coefficients, their `[General]` defaults, and `EnemyHouseThreatBonus` are zero unless a rules file sets them. If none is set, every candidate scores exactly `100000`, and the choice depends only on the order in which the scan reaches the candidates. The retail `rules.ini` sets the five `[General]` defaults and `EnemyHouseThreatBonus`, so a mod that replaces that file must set them itself.
:::

:::caution[A candidate already shooting at this object scores lower]
The first term changes sign when the candidate is targeting this object. Of two otherwise identical candidates, the one shooting at something else outscores the one shooting at this object by twice that term. A positive `TargetEffectivenessCoefficient` therefore draws fire toward dangerous candidates that are shooting at somebody else.
:::

:::caution[The distance term acts only outside weapon range]
`max(0, distance − range)` is zero for anything the object can already shoot, so `TargetDistanceCoefficient` does not separate candidates within range. Distance and range are both counted in cells, except in the ground part of a whole-map scan. There, distance is counted in leptons from the scan's center while range stays in cells. Almost every candidate there takes the penalty, about 256 times as strongly as on a ring scan.
:::

### Where the coefficients come from

When a type does not set a coefficient, the matching `[General]` setting supplies it: [`MyEffectivenessCoefficientDefault`](/keys/myeffectivenesscoefficientdefault/), [`TargetEffectivenessCoefficientDefault`](/keys/targeteffectivenesscoefficientdefault/), [`TargetSpecialThreatCoefficientDefault`](/keys/targetspecialthreatcoefficientdefault/), [`TargetStrengthCoefficientDefault`](/keys/targetstrengthcoefficientdefault/) and [`TargetDistanceCoefficientDefault`](/keys/targetdistancecoefficientdefault/). `SpecialThreatValue` has no such default.

:::caution[A per-type zero does not survive a later layer]
Each later rules layer that contains the type's section, such as the map, reads the coefficients again with the same fallback. A coefficient written as `0` is replaced by the `[General]` default at the first later layer whose copy of the section omits the key. An explicit `0` therefore lasts only until such a layer, unless the matching `[General]` default is also zero.
:::

```ini title="rules.ini"
[General]
TargetEffectivenessCoefficientDefault=1
TargetDistanceCoefficientDefault=-1

[MYTANK] ; example UnitType
MyEffectivenessCoefficient=2
TargetStrengthCoefficient=-0.5
```

`MYTANK` gives double weight to how much it can hurt a candidate, prefers wounded candidates, and takes the two `[General]` defaults for the coefficients it does not set. These are example values, not recommendations.

:::caution[`IsThreatRatingNode` has no effect]
The per-type coefficients are in force for every house from the first frame of every game. [`IsThreatRatingNode=yes`](/keys/isthreatratingnode/) was meant to switch them on when a house owned such a structure, so it changes nothing. The house-wide coefficients it would have replaced are listed under [settings without effect](#settings-without-effect).
:::

### Adjustments after scoring

These adjustments apply after the score is calculated, in this order:

1. If this house has been ordered to hunt with everything and has a declared enemy, any candidate outside that enemy scores exactly `1`.
2. A scan for power plants adds `Power × 1000` for a building with a positive [`Power`](/keys/power/), and sets any other building's score to zero.
3. A scan for factories sets the score of a building that produces nothing to zero.
4. A scan for base defenses sets the score of any candidate with no primary weapon to zero.
5. If this object's primary weapon sets [`Supress=yes`](/keys/supress/), the score is halved once for each cell near the candidate that holds a building of this house or an ally. The cells checked are the square rings around the candidate's cell, out to one cell less than the [`FireSupress`](/keys/firesupress/) distance in `[CombatDamage]`. A building that covers several of those cells halves the score several times. At `FireSupress=1` there is no ring to check, so the halving never happens until `FireSupress` is raised.
6. A score of exactly zero rejects the candidate. Any other score below `1` is raised to `1`.

## Which weapon the score assumes

The effectiveness terms and every range test in the rejection table use the weapon each side would choose against the other. Unless a web weapon is involved, the choice compares the two weapon slots:

- Each slot scores its warhead's `Verses` percentage against the target's armor, multiplied by `1000`.
- The score doubles when the target is within that slot's range.
- The score is zero when the slot cannot fire at the target at that moment: the target is illegal for it, it is reloading, or something else stops it firing, such as an unpowered building.
- The higher score wins, and a tie goes to the primary slot.

A web weapon replaces those scores. When either slot has a [`Webby=yes`](/keys/webby/) warhead and can fire at the target at that moment, the web slot is chosen against infantry that can be webbed and against most map cells. Infantry can be webbed unless it is [`IsWebImmune=yes`](/keys/iswebimmune/) or immobilized. The other slot is chosen against everything else.

A target that is not an object, such as a cell, counts as armor class `none`.

The primary slot does not always hold [`Primary`](/keys/primary/). An elite object uses its [`Elite`](/keys/elite/) weapon in that slot, and a building's plugged-in upgrades can replace the weapon in either case; [the elite weapon](/systems/veterancy/#the-elite-weapon) covers both. On this page, "primary weapon" means whatever weapon that slot holds.

:::danger[A web primary with no secondary reads an empty weapon slot]
When the primary's warhead is `Webby=yes` and the secondary slot is empty, the web rule above picks the empty secondary slot for any target the web weapon can fire at but cannot web. That covers every vehicle and building, a landed aircraft, and every `IsWebImmune=yes` or immobilized infantry. Scoring copes with the empty slot and uses `GuardRange` as the range. The retaliation check does not: when such an object is damaged by something its web weapon can fire at but cannot web, the check reads the warhead of the empty slot, which is an invalid memory access. Give any object with a web primary a secondary weapon.
:::

## Picking the winner

:::caution[A ring scan does not rank ground candidates]
A ring scan does not keep the highest score among ground candidates. Each ground candidate that passes replaces the one before it, so the scan returns the last one it reached, not the best one. Every Guard, Area Guard, Patrol, Move and building scan is a ring scan. On a ring scan, the coefficients matter in three places:

- they rank the aircraft and airborne infantry found by the flying-object passes below;
- a ground candidate must outscore the best of those to replace it;
- a score brought to exactly zero rejects the candidate.
:::

When aircraft are wanted, a ring scan first makes two passes over flying objects, and these passes do keep the highest score. The first pass takes aircraft and [`JumpJet=yes`](/keys/jumpjet/) infantry; the second takes any other infantry that is off the ground. Neither looks at vehicles, so a vehicle with a jumpjet locomotor is found by neither. A ground candidate then replaces the flying one only if it scores higher than the best flying score.

A ring scan can stop early. Once it has found an object, it stops at the end of the ring a quarter of the way out, or at the end of the ring halfway out. An object found beyond the halfway ring does not stop the scan, and later candidates keep replacing it.

Until a ring scan finds an object, it also looks for a wall to attack in each cell. Only computer-owned objects attack walls this way. A cell counts as a wall target only when **none of** these is true:

1. the difficulty sets [`DestroyWalls=no`](/keys/destroywalls/);
2. the cell has no overlay, or its overlay type is not [`Wall=yes`](/keys/wall/);
3. the wall is out of range of the weapon chosen against it;
4. this object has no primary weapon, or its primary weapon names no warhead;
5. the primary weapon's projectile sets `AG=no`;
6. the primary weapon's warhead is not `Wall=yes`;
7. the wall has no owner, or belongs to this house or an ally.

Only the third test uses the weapon chosen against the wall. The three after it read the primary slot, even when the secondary would fire, so a secondary weapon built to break walls never makes a wall a target. A nearer wall scores higher. The scan returns the best wall at the end of the ring where it found one, even if an object turns up later in that ring.

The whole-map scan is the only scan that ranks ground candidates. It checks aircraft and then every ground object, and keeps the highest score. When two candidates tie, the one reached first wins.

## Retaliation

Damage is answered without a scan. The engine tests the rows below in order, and the first row the object matches decides whether it retaliates.

| Condition | Retaliates |
| --- | --- |
| Human-owned and already has a target | No |
| The warhead is [`Veinhole=yes`](/keys/veinhole/) | Yes, unless the object is human-owned and has a movement destination |
| No attacker is known | No |
| The current mission sets [`Retaliate=no`](/keys/retaliate/) | No |
| The attacker is an ally | No |
| The object's weapons average zero damage or less, or it has no primary weapon | No |
| The chosen weapon's `Verses` against the attacker's armor is `0%` | No |
| The attacker is an aircraft and the chosen weapon's projectile is `AA=no` | No |
| Human-owned `C4=yes` infantry, or any human-owned object holding the `C4` ability, damaged by a building | No |
| Human-owned vehicle whose [`DeploysInto`](/keys/deploysinto/) type is [`Artillary=yes`](/keys/artillary/) | No |
| Human-owned object other than a building, with [`PlayerReturnFire=no`](/keys/playerreturnfire/) in `[CombatDamage]`, outside Guard, Area Guard and Patrol | No |
| Member of a `Suicide=yes` team | No |
| Computer-owned, and its current target scores higher than the attacker | No |
| Otherwise | Yes |

The computer-owned comparison uses the unrounded threat score, so a fraction can decide it.

An object that retaliates switches to attacking the attacker, whatever its mission. It does so at once when the attacker is in range. Out of range, a computer-owned object still retaliates, but a human-owned one does so only when the attacker is within [`Sight`](/keys/sight/) plus half a cell.

### A damaged building

A damaged building can also take its attacker as a target directly, without the table above. **All of** these must hold:

- the building is not being deconstructed;
- the attacker is not an ally;
- the building has a primary weapon whose projectile is not `AA=yes`;
- the building has no target, or its target is out of range.

The building then targets the attacker, unless the attacker is an aircraft, or the building is human-owned and [`PlayerReturnFire=no`](/keys/playerreturnfire/) is set. In those two cases it turns to a random facing instead.

### Scattering after damage

Only objects that can move scatter.

An object allowed to retaliate still scatters if it ends up with neither a target nor a destination. **Any of** these is enough:

- [`PlayerScatter=yes`](/keys/playerscatter/) in `[CombatDamage]`;
- the object holds the `SCATTER` ability.

Retaliating sets a target at once, so this happens only when an allowed retaliation produced no attack. There are two such cases: a `Veinhole=yes` warhead with no veinhole monster to turn on, and a human-owned object whose attacker is beyond both weapon range and `Sight` plus half a cell.

An object refused retaliation scatters when **all of** these hold:

- its current mission sets [`Scatter=yes`](/keys/scatter/#scope-mission-behavior);
- it is not tethered;
- it is not moving;
- it has neither a target nor a destination;
- it is not an aircraft;
- **any of:** the object is computer-owned; `PlayerScatter=yes` is set; it holds the `SCATTER` ability.

## Threat ratings that are not target selection

Two settings named for threat play no part in choosing a target.

[`ThreatPosed`](/keys/threatposed/) measures how dangerous an object is, not how attractive it is as a target. It sets the object's value when a team weighs its members, and feeds the per-region threat map each house keeps. It also sizes a computer house's [base defense response](/systems/base-attacked/#the-strength-budget), which multiplies the attacker's value by [`ComputerBaseDefenseResponse`](/keys/computerbasedefenseresponse/) in `[AI]`.

[`ThreatAvoidanceCoefficient`](/keys/threatavoidancecoefficient/) is used only for pathfinding, where it scales how much of the threat map a route may cross. A team's [`AvoidThreats=yes`](/keys/avoidthreats/) raises it to `1` for its members.

## Campaign and skirmish differences

- The undiscovered-target rejection applies only in campaigns.
- The `MultiplayPassive` rejection applies only outside campaigns. A [launch file](/formats/spawn-ini/) can lift it, and a neutral house is then scanned like any other. A player's units outside a team still ignore buildings that cannot shoot back, so they attack the armed part of a neutral base and leave the rest.
- An aircraft on Hunt first scans for Tiberium processors only outside campaigns.
- Harvester immunity comes from the multiplayer lobby's harvester truce outside campaigns, and from a scenario's `HarvesterImmune` setting in campaigns.
- Outside campaigns, [`MPLAYER.INI`](/formats/multiplayer-rules/) can rewrite a warhead's `Verses`, which changes every effectiveness term that uses that warhead.

## Settings without effect

Five house-wide coefficients are read and stored but never used, because every house uses the per-type coefficients instead: [`DumbMyEffectivenessCoefficient`](/keys/dumbmyeffectivenesscoefficient/), [`DumbTargetEffectivenessCoefficient`](/keys/dumbtargeteffectivenesscoefficient/), [`DumbTargetSpecialThreatCoefficient`](/keys/dumbtargetspecialthreatcoefficient/), [`DumbTargetStrengthCoefficient`](/keys/dumbtargetstrengthcoefficient/) and [`DumbTargetDistanceCoefficient`](/keys/dumbtargetdistancecoefficient/).

Two target kinds have no effect either. Nothing ever scans for civilians, and a scan that did would reject every candidate. `AG=yes` also adds a boats kind, but no object counts as a boat, so it neither widens nor narrows a scan.
