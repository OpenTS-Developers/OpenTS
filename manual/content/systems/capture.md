---
title: Engineers, capture and sabotage
summary: "Hands a structure or a vehicle to another house, restores it, spies on it, or arms it with a demolition charge, according to the soldier that walks in."
category: combat-targeting
keys:
  - Agent
  - BridgeRepairHut
  - C4
  - C4Delay
  - C4Warhead
  - Capturable
  - ConditionRed
  - Crewed
  - Engineer
  - EngineerCaptureLevel
  - Infiltrate
  - Insignificant
  - IsMobileWar
  - Repairable
  - Strength
  - SurvivorDivisor
  - SurvivorRate
  - Thief
  - VehicleThief
related:
  - type: system
    id: repair
  - type: system
    id: veterancy
  - type: system
    id: target-selection
---

Almost everything below turns on which [mission](/systems/target-selection/#missions-in-brief) a soldier is in — the state that says what it is doing at this moment, and the rules section named after it. Target selection introduces both.

## Who can walk in

Six settings on an InfantryType decide what a soldier does when it reaches whatever it was sent at.

| Setting | What the soldier does on arrival |
| --- | --- |
| [`Engineer=yes`](/keys/engineer/#scope-infantrytype) | Restores an allied structure, takes or damages a non-allied one, repairs a bridge |
| [`C4=yes`](/keys/c4/) | Arms a [`Repairable=yes`](/keys/repairable/) structure with a demolition charge and walks away |
| [`Agent=yes`](/keys/agent/) | Infiltrates a structure and reports what it holds |
| [`Infiltrate=yes`](/keys/infiltrate/) | Nothing on its own. It is what lets a soldier be given a structure as a target and a destination at all |
| [`VehicleThief=yes`](/keys/vehiclethief/) | Takes the vehicle it was sent at |
| [`Thief=yes`](/keys/thief/) | Takes a non-allied vehicle it is walking toward, under any mission |

:::caution[`Infiltrate=no` cannot be written on an engineer or a demolition type]
`C4=yes` and `Engineer=yes` each force the infiltrate flag on, and they do it immediately after the section's own `Infiltrate` line has been read. An explicit `Infiltrate=no` in such a section is overwritten in the same pass and changes nothing. The forcing also outlives the flag that caused it: a later rules layer that writes `Engineer=no` over an engineer type clears the engineer behavior but leaves the infiltrate behavior in place, so the type still takes the enter cursor over a `Capturable=yes` structure, still walks in, and does nothing when it arrives. `Agent=yes` forces nothing, so a spy needs an explicit `Infiltrate=yes` or it is never offered a structure at all.
:::

## The cursor

### An engineer over a structure

A **deployed vehicle** is a structure that names an [`UndeploysInto`](/keys/undeploysinto/) vehicle and is not a construction yard. The engine counts one as a vehicle rather than as a structure, and that distinction runs through the rest of this page.

A player-controlled engineer has cursor rules of its own. They apply to a structure meeting **all of**:

- **Any of:** it is not a deployed vehicle; it is [`IsMobileWar=yes`](/keys/ismobilewar/).
- **Any of:** it is [`Repairable=yes`](/keys/repairable/); it is `IsMobileWar=yes`.

An `IsMobileWar=yes` structure therefore reaches these rules whatever else it is, and every other structure has to clear both groups. For one that does, the table gives the cursor produced.

| Structure | Cursor |
| --- | --- |
| A [`BridgeRepairHut=yes`](/keys/bridgerepairhut/) type | The repair cursor when a bridge near it can be repaired, the refusal cursor otherwise |
| An allied structure at full [`Strength`](/keys/strength/#scope-aircrafttype) | The refusal cursor |
| An allied structure below full strength | The repair cursor |
| A non-allied [`Capturable=yes`](/keys/capturable/) structure above [`EngineerCaptureLevel`](/keys/engineercapturelevel/) | The damage action, which carries no cursor art of its own |
| A non-allied `Capturable=yes` structure at or below that fraction | The enter cursor |

The full-strength test compares against a figure fixed at `1` in the engine and reads the strength live, so the repair cursor appears the moment an allied structure drops a single point.

Anything these rules do not settle falls through to the general rewrite every `Infiltrate=yes` soldier shares — both a structure the two groups above rejected and a non-allied structure that clears them but is not `Capturable=yes`, which matches no row. That rewrite offers the enter cursor over a non-allied `Capturable=yes` structure with no strength test at all. That is the route a `Repairable=no` structure takes — a barrel, a mine, a wall — and the route a spy takes for everything. It withholds the cursor when no cell adjacent to the structure's footprint sits in the same [movement zone](/glossary/#movement-zone) as the soldier, which is what keeps it off a naval yard that no land route reaches.

:::caution[The capture threshold moves the cursor and nothing else]
`EngineerCaptureLevel` picks between the enter cursor and a damage action, and the click folds the repair cursor, the damage action and the enter cursor into a single capture order before it is issued. What happens at the structure is decided again from the flags found there. The damage action has no cursor art of its own and falls back to the ordinary pointer, and at the engine default of `1` it cannot be produced at all, because a structure's strength as a fraction of its maximum can never rise above `1`.
:::

With fog of war switched on, an engineer's cursor over a cell that holds a fogged structure record ignores ownership, strength and `Capturable` alike: a bridge repair hut takes the repair or refusal cursor and any `Repairable=yes` type takes the enter cursor. That click issues the capture order against the cell rather than against an object.

### A commando over a structure

For a player-controlled infantry with `C4=yes` or the [`C4` ability](/systems/veterancy/#abilities), an attack cursor over a structure that is not a deployed vehicle becomes the demolitions cursor when the type is `Repairable=yes` and stays an attack cursor when it is not — which is how a commando shoots a barrel and bombs everything else. Force fire reaches an allied or own structure the same way, because holding it produces the attack cursor there in the first place.

### A vehicle thief over a vehicle

A player-controlled `VehicleThief=yes` soldier takes the enter cursor over any non-structure counted as a vehicle, which includes a landed aircraft. An [`IsTrain=yes`](/keys/istrain/) type takes the select cursor instead. The ownership test is house identity rather than alliance, so the cursor appears over an allied house's vehicle as readily as over an enemy's.

:::caution[The harvester truce never protects a vehicle from a thief]
The clause meant to exempt a harvester while the truce is on tests the vehicle standing on the map for membership in [`HarvesterUnit`](/keys/harvesterunit/), which lists types rather than the objects made from them. A runtime vehicle can never be equal to a type definition, so the test always comes out false, the exemption never fires, and the enter cursor appears over a harvester under every setting. The equivalent exemption in [target selection](/systems/target-selection/#why-a-candidate-is-rejected) reads the vehicle's type instead, and that one does hold a harvester off.
:::

### What the click issues

The three sections above each settle a cursor. This table takes over from there: the left column is what those rules produced and the right is the order a click on it issues. Three of them share the first row, which is why what happens at the structure is settled again on arrival rather than by the cursor the player saw.

| What the cursor test produced | Order issued |
| --- | --- |
| Repair, enter, or the damage action, over an object | The [capture mission](/reference/enums/mission/), with the object as the destination |
| Demolitions | The sabotage mission, with the object as the destination |
| Enter, over a cell | The capture mission, with the cell as the destination |
| Area guard, over a non-allied `Repairable=yes` structure, from a `C4=yes` soldier | The sabotage mission |
| Refusal | Nothing |

Both missions are serviced on the cadence their own [`Rate`](/keys/rate/#scope-mission-behavior) sets in the `[Capture]` and `[Sabotage]` sections, plus a jitter of up to two frames. A vehicle handed either order converts it to an attack. An `Infiltrate=yes` soldier given a structure as an attack target converts that order into the capture mission, and one with `C4=yes` or the `C4` ability converts it into sabotage first only when the structure is `Repairable=yes`.

## Walking in

Everything in this section happens when the soldier finishes a move onto a cell while on the capture, area guard or patrol mission. Area guard and patrol are in that gate, which is how a computer engineer restores a structure without an explicit order.

An infantry on the capture, sabotage or enter mission may step into the cell holding its destination, and an engineer may do so on guard, area guard and patrol as well; a vehicle thief gets the same allowance for any non-`IsTrain=yes` vehicle it is aimed at. Under the capture, enter, area guard and patrol missions the walking locomotor will also let it claim a sub-cell position — one of the three standing places a [cell](/glossary/#cell) offers infantry — that another soldier already holds. An `Infiltrate=yes` type is exempt from the check that would otherwise discard a destination lying in another [movement zone](/glossary/#movement-zone).

### The vehicle branch

The first test is on the destination alone: a live object counted as a vehicle. There is no flag test, no alliance test and no `Capturable` test. While the soldier is off the cell, the destination is simply re-aimed at the vehicle. Once the two stand on the same cell, this runs in order:

1. the entered trigger on the vehicle springs;
2. the vehicle detaches from everything tracking it;
3. ownership changes;
4. infantry heading into the vehicle scatter, which happens only where it is in fact a deployed vehicle;
5. the soldier's tag moves onto the vehicle, if that tag is transferable;
6. the soldier's own destroyed-anything trigger springs;
7. the soldier is deleted.

A soldier's **tag** is the scenario tag attached to it — the record that binds a trigger to an object. It is **transferable** when the trigger behind it, or any trigger linked behind that one, sets the transferable field of its own record. A tag that is not transferable does not move; step 7 deletes the soldier still carrying it.

:::caution[An engineer takes a mobile war factory outright]
A deployed vehicle counts as a vehicle here. Because this branch runs ahead of all the engineer handling and applies none of its tests, an engineer that reaches a deployed mobile war factory changes its owner on the spot — at any strength, whatever `Capturable` says, and including an allied one. The allied case is reachable straight from the cursor: a damaged allied `IsMobileWar=yes` structure offers the engineer's repair cursor, and that click issues the same capture order.
:::

### Restoring an allied structure

An engineer that reaches an allied structure restores it to full strength at no cost, and [that also forces the repair flag off](/systems/repair/#what-stops-a-repair). The damage animation state is re-evaluated on the new strength. Nothing else about the structure changes.

### Repairing a bridge

A `BridgeRepairHut=yes` structure replaces both the restore and the capture branches, so a hut is never restored and never changes hands whoever owns it. EVA announces the repair for a player-controlled house. The repair to run is chosen from the five-by-five block of cells centered on the soldier: a rail bridge tile anywhere inside it selects the rail repair, and the road repair runs otherwise. Every infantry then drops the hut as a target, and infantry heading into the cell scatter.

### Capturing a non-allied structure

For a non-allied `Capturable=yes` structure the entered trigger springs again, the losing house is marked as having been robbed, the soldier's tag is attached to the structure if it is transferable, ownership changes, and infantry heading into the cell scatter.

That mark is the one the [Thieved by...](/mapping/events/tevent-thieved/) trigger event tests. It is set here and nowhere else — not by a stolen vehicle — and nothing ever clears it, so once a house has lost one structure to an engineer the mark stands for the rest of the match. Nothing acts on it, though: the event is never announced to a trigger and so is never satisfied, and no other part of the game reads the mark.

### Damaging it instead

The engineer damages the structure instead of taking it under **all of**:

- the game is not a campaign game;
- the lobby's multiplayer engineer option is on;
- the structure's strength fraction is above [`ConditionRed`](/keys/conditionred/).

`Capturable` is not consulted on this branch.

```text title="damage one engineer deals"
damage = min(Strength − MaxStrength × ConditionRed / 2,
             MaxStrength × (1 − ConditionRed / 2) / 2)
```

The figure is applied as forced damage with [`C4Warhead`](/keys/c4warhead/) and the engineer as the source. Forced damage skips the warhead's own effectiveness table, the house and object armor biases, the veteran armor bonus and [`Immune=yes`](/keys/immune/) alike, so the structure takes the whole figure. At the default `ConditionRed` of `0.5` that is a bite of at most 37.5% of maximum strength, and the branch stops as soon as the structure is at or below half strength:

| Engineer | Strength before | Strength after |
| --- | --- | --- |
| First | 100% | 62.5% |
| Second | 62.5% | 25% |
| Third | 25% | Captured |

`ConditionRed` therefore sets both the band in which the branch runs and the size of the step, and at `1` or above the branch is unreachable, since the strength fraction can never exceed it.

:::danger[This branch reads `C4Warhead` without checking it]
A structure that crosses the half-strength or the red-strength mark runs the fire-and-smoke handling, which reads the warhead it was damaged with and never tests it first. `C4Warhead` holds nothing until `[CombatDamage]` names one, so with the setting absent the second engineer's hit — the one that crosses both marks — reads through a warhead that was never supplied. Name a `C4Warhead` before switching the multiplayer engineer option on.
:::

### Infiltrating it

A soldier that is not an engineer does anything at the structure only when it is `Agent=yes`. EVA announces the infiltration for a player-controlled house, and the house is recorded as spying on that structure: a spied structure shows its status overlay whenever the spying player selects it, and a spied factory also draws its current product through the same cameo palette as the sidebar.

A [`Radar=yes`](/keys/radar/) structure adds far more. Spying on one marks the whole owning house as radar-spied, and while that mark stands every sight the victim's objects take is credited to the local player instead — a standing share of the victim's vision rather than a single reveal. The mark is recomputed only when a spied radar structure is destroyed or changes hands, and capturing a structure clears the capturing house's own spy mark on it. Spying on a structure with positive [`Power`](/keys/power/#scope-buildingtype) adds nothing but a redraw of whichever spied structures the player has selected at that moment.

### The soldier is consumed

Every path through the structure branch deletes the soldier, including the paths where nothing at all happens: an engineer at a non-capturable non-allied structure inside a campaign, and any soldier that is neither an engineer nor `Agent=yes`. Arming a demolition charge is the one entry path on this page that leaves the soldier alive.

## What changes hands

Change of ownership is one shared routine, and it runs the same way for a structure, a vehicle and an aircraft.

- The object is booked as a kill with no source attached. The capturing house collects the score points and [nothing gains experience](/systems/veterancy/#earning-experience); the object keeps the rank it was already carrying.
- The losing house's inventory counts fall and the new owner's rise, except on an [`Insignificant=yes`](/keys/insignificant/#scope-aircrafttype) type, which changes hands without moving between the two tallies at all.
- The object's current target and destination are cleared, it detaches from everything tracking it, cell threat moves from one house to the other, and it re-enters idle behavior.

A structure adds to that:

- The storage capacity its [`Storage`](/keys/storage/) declares moves to the new owner, and so does whatever the structure is holding — straight into the new owner's spendable credits. The loser simply loses it; it is not booked as harvested on the way out.
- Anything loaded inside is captured with it. So is anything in radio contact at a weapons factory, or within a quarter of a cell of the docking coordinate; anything further off is told to run away and contact is broken.
- Production in progress is abandoned. The object under construction is deleted and the money spent on it so far is refunded to the losing house.
- Both houses recount their factories, and a captured construction yard moves between their construction-yard lists. A player who has just lost the last of them also loses any placement cursor in progress.
- The structure re-opens for business as a capture rather than as a new build, which skips the two gifts a fresh structure hands out: a captured [`FreeUnit`](/keys/freeunit/) structure yields no free vehicle and a captured helipad no free aircraft.
- The repair flag is cleared, a cloak generator's radius is reset and re-enabled if it still has power, and EVA announces the capture when either side is under player control.
- The losing house records the structure as its recapture target, which is what its own engineers head for.

:::caution[A computer house sells what it captures]
Outside a campaign game, a capture by a non-human house refunds and removes every upgrade plugged into the structure and then sells the structure itself. The only exception is a structure that produces buildings and that the capturing house now owns at most one of. A type with no build-up animation cannot be sold, so there the upgrades are still stripped and refunded and the bare structure stays — except a firestorm wall section, which is removed outright with no refund.
:::

Capture leaves three lasting marks besides the change of owner. A captured structure halves the number of survivors it will ever produce and worsens their odds, it never returns the `[General]` [`Engineer`](/keys/engineer/#scope-global-rules) type among its crew whether it is destroyed or sold, and it satisfies [the computer's last repair gate](/systems/repair/#when-the-computer-repairs) on its own, without the structure having been marked for repair. Nothing clears the captured mark, so recapturing it back undoes none of them.

Rank, abilities and the elite weapon [survive the change intact](/systems/veterancy/#carrying-rank-between-objects). Strength is neither restored nor reduced. And an armed demolition charge keeps its countdown running through the whole thing.

## Demolition charges

### Arming a structure

Arming requires the sabotage mission, a `Repairable=yes` structure as the destination, and the type still carrying that flag when the soldier arrives. A sabotage mission whose destination is a structure without that flag — stale or assigned directly — is canceled, and the soldier is put back on guard. There is no alliance test, so force fire can still send a saboteur against a qualifying structure belonging to its own house. Unless the structure is already deconstructing, the charge is set, the structure begins flashing as a designated target, the countdown is set to [`C4Delay`](/keys/c4delay/) minutes' worth of frames — 27 frames, about 1.8 seconds, at the default — and the saboteur is recorded against it. The saboteur then clears its destination, uncloaks, takes a rearm delay and scatters away from the structure's center.

### Detonation

When the countdown reaches zero the structure takes damage equal to its current strength, applied as forced damage with `C4Warhead` and sourced to the saboteur — or to nobody, if the saboteur died first, which also costs its house the kill credit.

:::danger[A charge cannot be defused]
Nothing anywhere clears the armed state. Repairing the structure, restoring it with an engineer and capturing it all leave the charge armed and the countdown running. A sale requested explicitly — by a computer house, by a trigger action, or by the capture sell-off above — is refused while a charge is armed, but a player's own sell order does not consult it and begins deconstruction with the countdown still going.
:::

### Survivors

The count is `Cost × SurvivorRate / SurvivorDivisor`, clamped to between 1 and 5, doubled in the divisor for a captured structure, and zero for a type that is not [`Crewed=yes`](/keys/crewed/). Each cell of the footprint then rolls once for one survivor, at odds set by two independent facts about the structure: whether it was ever captured, and whether a live saboteur is on record against it. Arming a charge is what puts a saboteur on record, and the saboteur's own death takes it off again.

| Roll per footprint cell | No saboteur on record | Saboteur on record |
| --- | --- | --- |
| Never captured | One in three | One in two |
| Captured | One in nine | One in eight |

Being captured is by far the heavier of the two: it costs more than the saboteur gains, so a captured structure is a poorer source of survivors even with a charge ticking on it. The survivors attack the saboteur on record if there is one; otherwise a human house's wander off and a computer house's go hunting.

A structure destroyed by its own charge produces none of them. The detonation is forced damage, and a forced destruction is marked survivorless before the count is taken, so the improved right-hand column is only ever reached on a structure that something else finished off first — and only while the saboteur is still alive to hold its place on the record.

## Stealing a vehicle

`VehicleThief=yes` and `Thief=yes` are separate features with separate cursors, targeting and limits. They meet in one place: both stamp the vehicle with the type that took it, which is what puts a hijacker back on the map when the stolen vehicle dies.

A `VehicleThief=yes` soldier is the one with a cursor. It takes the vehicle through [the vehicle branch](#the-vehicle-branch), and a computer house turns its hunt into a capture order and leaves it on area guard when idle. Two consequences outlive the theft:

- The stolen vehicle counts against the thief type's [`BuildLimit`](/keys/buildlimit/) for as long as it lives, so a hijacker limited to one cannot be rebuilt while its prize survives.
- When that vehicle is destroyed, the hijacker steps back out. It is recreated at the wreck with a strength between 5 and half its maximum, with no `Crewed=yes` requirement and no crew-escape roll to pass, hunting for a computer house and standing guard for a human one.

A vehicle thief also loses its target outright when the vehicle it is chasing deploys into a mobile war factory or a construction yard, while every other object chasing that vehicle is handed the new structure instead.

`Thief=yes` has no cursor anywhere. Its steal runs on every one of the soldier's own passes regardless of mission, and needs a non-allied vehicle as the movement destination: within half a cell and one height level the vehicle's entered trigger springs, radio contact breaks, the thief's tag moves across if it is transferable, ownership changes and the thief is deleted. Beyond that it re-aims at the vehicle whenever the vehicle wanders off. The flag also [widens what its owner scans for](/systems/target-selection/#what-each-kind-of-object-considers).

## Computer-controlled engineers

A computer house aims engineers through [target selection](/systems/target-selection/#what-each-kind-of-object-considers), which covers the scanning rules, the fifteen-cell shortcut to the house's recapture target, and the filter that lets an engineer accept a damaged ally and reject everything else. Four mission conversions sit outside that:

- A hunt becomes a capture order for an engineer that is neither `C4=yes` nor carrying the `C4` ability, and for a vehicle thief.
- A hunt becomes a sabotage order for a `C4=yes` soldier whose target is a `Repairable=yes` structure.
- Guard and area guard become a sabotage order under the same condition, for a computer house only.
- A patrolling engineer drops an allied structure as a target once that structure climbs back above `ConditionRed`.

An engineer skips target scanning altogether on plain guard, and being shot at while standing on guard or area guard sends a computer-owned one hunting.

## Settings without effect

[`EngineerDamage`](/keys/engineerdamage/) is read from `[General]` under its own name, but no gameplay path consults the figure it fills, so writing it changes nothing.

The sabotage handling has a second branch, for a saboteur whose destination is a bare cell rather than a structure, that fires three area explosions of [`BridgeStrength`](/keys/bridgestrength/) with `C4Warhead` at the soldier's own position. No cursor produces that combination, because the cell branch of the click handler is never reached. The computer's guard and area-guard conversions do not clear an existing destination, so a computer-owned saboteur can carry the mission with a cell destination and reach it that way.
