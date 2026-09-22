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
  - type: system
    id: produce-cash
---

Most of this page depends on the soldier's current [mission](/systems/target-selection/#missions-in-brief), such as capture, sabotage or area guard. A mission is what the soldier is doing at the moment. Each mission also has a rules section of the same name, such as `[Capture]`, that controls it, including how often it runs. Target selection introduces both.

## Who can walk in

Six settings on an InfantryType decide what a soldier does when it reaches whatever it was sent at.

| Setting | What the soldier does on arrival |
| --- | --- |
| [`Engineer=yes`](/keys/engineer/#scope-infantrytype) | Restores an allied structure, takes or damages a non-allied one, repairs a bridge |
| [`C4=yes`](/keys/c4/) | Arms a [`Repairable=yes`](/keys/repairable/) structure with a demolition charge and walks away |
| [`Agent=yes`](/keys/agent/) | Spies on the structure it enters |
| [`Infiltrate=yes`](/keys/infiltrate/) | Lets the soldier be sent into a structure. On its own, the soldier is consumed on arrival and nothing else happens |
| [`VehicleThief=yes`](/keys/vehiclethief/) | Takes the vehicle it was sent at |
| [`Thief=yes`](/keys/thief/) | Takes a non-allied vehicle it is walking toward, under any mission |

:::caution[`Infiltrate=no` cannot be written on an engineer or a demolition type]
`C4=yes` and `Engineer=yes` each turn `Infiltrate` on after the section is read, so an explicit `Infiltrate=no` in the same section changes nothing. The forced value also stays after the flag that caused it is gone. If a later rules layer writes `Engineer=no` over an engineer type and does not also write `Infiltrate=no`, the type loses its engineer behavior but keeps `Infiltrate`. It still takes the enter cursor over a `Capturable=yes` structure, still walks in, and is consumed with no effect. `Agent=yes` forces nothing, so a spy needs an explicit `Infiltrate=yes` before it can be sent into a structure.
:::

## The cursor

### An engineer over a structure

A **deployed vehicle** is a structure whose type names an [`UndeploysInto`](/keys/undeploysinto/) vehicle and is not a construction yard. The engine counts a deployed vehicle as a vehicle, not as a structure, and that distinction runs through the rest of this page.

A player-controlled engineer has separate cursor rules for a structure that meets **all of**:

- **Any of:** it is not a deployed vehicle; it is [`IsMobileWar=yes`](/keys/ismobilewar/).
- **Any of:** it is [`Repairable=yes`](/keys/repairable/); it is `IsMobileWar=yes`.

An `IsMobileWar=yes` structure therefore always reaches these rules, and any other structure must pass both groups. For a structure that does, the table gives the cursor.

| Structure | Cursor |
| --- | --- |
| A [`BridgeRepairHut=yes`](/keys/bridgerepairhut/) type | The repair cursor when a bridge near it can be repaired, the refusal cursor otherwise |
| An allied structure at full [`Strength`](/keys/strength/#scope-aircrafttype) | The refusal cursor |
| An allied structure below full strength | The repair cursor |
| A non-allied [`Capturable=yes`](/keys/capturable/) structure whose strength fraction is above [`EngineerCaptureLevel`](/keys/engineercapturelevel/) | The damage action, which has no cursor art |
| A non-allied `Capturable=yes` structure at or below that fraction | The enter cursor |

Full strength means exactly maximum strength, so the repair cursor appears as soon as an allied structure loses one point.

Two cases get no cursor from these rules: a structure that failed either group above, and a non-allied structure that passes both but is not `Capturable=yes`. Both fall through to the rule every `Infiltrate=yes` soldier uses, including a spy. That rule gives the enter cursor over any non-allied `Capturable=yes` structure whose type is [`LegalTarget=yes`](/keys/legaltarget/), whatever its strength. This is how an engineer is offered a `Repairable=no` structure such as a barrel, a mine or a wall, if its type is `Capturable=yes`.

The shared rule shows the no-move cursor instead when the soldier cannot walk up to the structure. That happens when no cell next to the structure's footprint is in the soldier's [movement zone](/glossary/#movement-zone), as with a naval yard that no land route reaches.

:::caution[The capture threshold changes only the cursor]
`EngineerCaptureLevel` chooses between the enter cursor and the damage action. A click on either one, or on the repair cursor, gives the same capture order. When the engineer arrives, the structure's current flags decide what happens, not the cursor the player saw. The damage action has no cursor art, so it shows the ordinary pointer. At `EngineerCaptureLevel=1` the damage action never appears, because a structure's strength fraction never rises above `1`.
:::

With fog of war on, an engineer over a cell holding a fogged structure record gets a cursor that ignores ownership, strength and `Capturable`. A bridge repair hut gives the repair or refusal cursor, and any `Repairable=yes` type gives the enter cursor. The click itself does not use this rule: it gives whatever order the cell would give without the fogged record, so it never issues a capture or bridge repair order.

### A commando over a structure

A player-controlled soldier with `C4=yes` or the [`C4` ability](/systems/veterancy/#abilities) gets the demolitions cursor in place of the attack cursor over a `Repairable=yes` structure that is not a deployed vehicle. Over any other structure the attack cursor stays, but the attack order [turns into a capture order](#what-the-click-issues). The commando then walks up to the structure and is consumed when it arrives, unless the structure is destroyed first. Force fire gives the attack cursor over an allied or own structure too, so the same rule lets a commando bomb a `Repairable=yes` structure of its own side.

A `C4=yes` soldier also gets the demolitions cursor over a non-allied `Capturable=yes`, `LegalTarget=yes` deployed vehicle. The click gives an order only when that type is also `Repairable=yes`.

### A vehicle thief over a vehicle

A player-controlled `VehicleThief=yes` soldier gets the enter cursor over any vehicle owned by another house. That includes a landed aircraft but not a deployed vehicle. A [`NonVehicle=yes`](/keys/nonvehicle/) type does not count as a vehicle. An [`IsTrain=yes`](/keys/istrain/) type gives the select cursor instead. The test compares houses, not alliances, so an allied player's vehicle gets the enter cursor too.

:::note[The harvester truce withholds the enter cursor]
While the [`HarvesterImmune`](/keys/harvesterimmune/) truce is on, a vehicle whose type is listed in [`HarvesterUnit`](/keys/harvesterunit/) gives the select cursor instead of the enter cursor. [Target selection](/systems/target-selection/#why-a-candidate-is-rejected) applies the same exemption when it rejects a candidate.
:::

### What the click issues

A click on an object gives an order according to the cursor the rules above produced. The area guard row applies to the area guard order, given by holding both force-fire and force-move.

| Cursor | Order issued |
| --- | --- |
| Repair, enter, or the damage action | The [capture mission](/reference/enums/mission/), with the object as the destination |
| Demolitions | The sabotage mission, with the structure as the destination, when the structure is `Repairable=yes` |
| Area guard, over a non-allied `Repairable=yes` structure, from a soldier with `C4=yes` or the `C4` ability | The sabotage mission |
| Refusal | Nothing |

The repair cursor gives an attack order instead when the soldier's weapon heals.

Both missions run at the interval their [`Rate`](/keys/rate/#scope-mission-behavior) sets in the `[Capture]` and `[Sabotage]` sections, plus up to two extra frames at random.

An attack order against a structure can also turn into one of these missions. A soldier with `C4=yes` or the `C4` ability switches to sabotage when the structure is `Repairable=yes`. Otherwise an `Infiltrate=yes` soldier switches to capture.

## Walking in

Everything in this section happens when a soldier finishes a move onto a cell while on the capture, area guard or patrol mission. Area guard and patrol are included, which is how a computer engineer restores a structure without an explicit order.

A soldier on the capture, sabotage or enter mission may step into the cell holding its destination or its target. An engineer may also do so on guard, area guard and patrol. A vehicle thief may step into the cell of any non-`IsTrain=yes` vehicle that is its destination.

A [cell](/glossary/#cell) has three standing places for infantry. On the capture, enter, area guard and patrol missions, a soldier walking into its destination's cell may take a standing place that another soldier already holds.

An ordinary soldier drops a destination in another [movement zone](/glossary/#movement-zone). An `Infiltrate=yes` type keeps it, and so does any soldier on the enter mission.

When a soldier walks into its target structure, the structure's entered trigger springs first, whatever kind of soldier it is.

### The vehicle branch

This branch applies to any live destination the engine counts as a vehicle, including a deployed vehicle. It checks no flag on the soldier, no alliance and no `Capturable`. While the soldier is on a different cell, it keeps heading for the vehicle. Once both stand on the same cell, the following happens in order:

1. the vehicle's entered trigger springs;
2. everything tracking the vehicle drops it;
3. the vehicle changes owner, which springs its entered trigger again;
4. infantry heading into a deployed vehicle scatter;
5. the soldier's tag moves onto the vehicle, if the tag is transferable;
6. the soldier's destroyed-anything trigger springs;
7. the soldier is deleted.

A soldier's **tag** is the scenario tag attached to it, which links it to a trigger. The tag is **transferable** when its trigger, or any trigger linked after it, sets [field 8 of its trigger row](/formats/scenario-triggers/#the-trigger-row), which hands the tag on. A tag that is not transferable does not move to the vehicle.

A vehicle that already belongs to the soldier's house does not change owner, but the soldier is still consumed.

:::caution[A deployed vehicle is taken outright]
A deployed vehicle counts as a vehicle here, so this branch runs before any engineer handling and applies none of its tests. A soldier that reaches another house's deployed vehicle on the capture mission takes it at any strength, whatever its `Capturable` setting. Two cursors let a player order this:

- Any `Infiltrate=yes` soldier without `C4=yes`, a spy included, gets the enter cursor over a non-allied `Capturable=yes`, `LegalTarget=yes` deployed vehicle.
- An engineer gets the repair cursor over a damaged allied `IsMobileWar=yes` structure. That click gives the same capture order, so the engineer takes the allied player's mobile war factory.
:::

### Restoring an allied structure

An engineer that reaches an allied structure restores it to full strength at no cost. It also [switches the structure's repair off](/systems/repair/#what-stops-a-repair). The structure's damaged appearance updates to the new strength. Nothing else about the structure changes.

### Repairing a bridge

An engineer that reaches a `BridgeRepairHut=yes` structure repairs a bridge, whoever owns the hut. The hut is never restored and never changes hands. EVA announces the repair when the engineer belongs to a player-controlled house.

The engineer's position decides the kind of repair. A rail bridge tile anywhere in the five-by-five block of cells centered on the engineer selects the rail bridge repair. Otherwise the road bridge repair runs. Every infantry then stops targeting the hut, and infantry heading into its cell scatter.

### Capturing a non-allied structure

An engineer that reaches a non-allied `Capturable=yes` structure takes it. The structure's entered trigger springs again, and the losing house is marked as robbed. The soldier's tag moves onto the structure if it is transferable. Ownership then changes, which springs the entered trigger a third time, and infantry heading into the cell scatter.

The robbed mark is what the [Thieved by...](/mapping/events/tevent-thieved/) trigger event tests. Only this capture sets it; a stolen vehicle does not. Nothing clears it, so once a house loses one structure to an engineer, the mark stays for the rest of the match. A trigger that uses this event still [never fires](/systems/trigger-springing/#three-events-that-cannot-be-reached), and no other part of the game reads the mark.

### Damaging it instead

An engineer damages a non-allied structure instead of taking it when **all of** these hold:

- the game is not a campaign game;
- the multiplayer engineer option is on (`MultiEngineer` in a [spawn file](/formats/spawn-ini/));
- the structure's strength fraction is above [`ConditionRed`](/keys/conditionred/).

This branch does not read `Capturable`, so it also damages a structure the engineer could never take.

```text title="damage one engineer deals"
damage = min(Strength − MaxStrength × ConditionRed / 2,
             MaxStrength × (1 − ConditionRed / 2) / 2)
```

The damage uses [`C4Warhead`](/keys/c4warhead/) and credits the engineer. It is forced damage, so the warhead's effectiveness against the structure's armor, the house and object armor biases, the veteran armor bonus and [`Immune=yes`](/keys/immune/) do not reduce it.

With `ConditionRed=0.5`, each engineer removes at most 37.5% of maximum strength, and damage stops once the structure is at or below half strength:

| Engineer | Strength before | Strength after |
| --- | --- | --- |
| First | 100% | 62.5% |
| Second | 62.5% | 25% |
| Third | 25% | Captured, if `Capturable=yes` |

`ConditionRed` sets both the strength range where engineers deal damage and the size of each hit. At `1` or above, engineers never deal damage, because the strength fraction cannot exceed it.

:::danger[Set `C4Warhead` before turning on the multiplayer engineer option]
When a structure drops below half strength or below `ConditionRed`, the game reads the damaging warhead to choose fire effects. `C4Warhead` has no warhead until `[CombatDamage]` names one. Without it, the first engineer hit that crosses either mark crashes the game. With `ConditionRed=0.5` that is the second engineer.
:::

### Infiltrating it

A soldier that is not an engineer has an effect at the structure only when it is `Agent=yes`. EVA announces the infiltration when the spy belongs to a player-controlled house, and the spy's house is recorded as spying on that structure. There is no alliance test. Capturing a structure clears the capturing house's spy record on it.

When the spying player selects a spied structure, it shows the same status display and health pips as an allied structure. A spied factory also shows the cameo of whatever it is producing.

Spying on a [`Radar=yes`](/keys/radar/) structure also marks the whole owning house as radar-spied. While that mark stands, the spying player sees whatever the victim's objects see. The mark is recalculated only when a spied radar structure is destroyed, or captured by a house that had spied on it.

Spying on a structure with positive [`Power`](/keys/power/#scope-buildingtype) has no further effect.

### The soldier is consumed

Every soldier that walks into its target structure is deleted, even when nothing happens there. That includes an engineer at a non-allied structure that is not `Capturable=yes` in a campaign, and any soldier that is neither an engineer nor `Agent=yes`. Arming a demolition charge is the only arrival on this page that leaves the soldier alive.

## What changes hands

A structure, a vehicle and an aircraft change owner through the same steps.

- The object's entered trigger springs.
- The object is recorded as a kill with no attacker. The capturing house gains score points equal to the object's cost, and [nothing gains experience](/systems/veterancy/#earning-experience). The object keeps its current rank.
- The object moves from the losing house's inventory counts to the new owner's. An [`Insignificant=yes`](/keys/insignificant/#scope-aircrafttype) type is kept out of those counts, so nothing moves.
- The object's target and destination are cleared, and everything tracking it drops it. Its threat on the map moves from one house to the other.

A captured structure also goes through these steps:

- The storage capacity from its [`Storage`](/keys/storage/) moves to the new owner. So does the Tiberium stored in it: the losing house loses it, and the new owner gains it at once without it counting as harvested.
- Anything loaded inside is captured with it. An object in radio contact is captured too if the structure is a weapons factory or the object is within a quarter of a cell of the docking point. Any other object in radio contact is told to move away, and contact is broken.
- Production in progress is abandoned. The object under construction is removed, and the money spent on it so far is refunded to the losing house.
- Both houses recount their factories. A captured construction yard moves between the two houses' lists of construction yards. If the player loses their last construction yard this way, any structure placement in progress is canceled.
- The structure opens as a capture, not as a new build. It therefore gives out no free units: a captured [`FreeUnit`](/keys/freeunit/) structure yields nothing, and a captured helipad gets no free aircraft.
- A structure that produces cash follows the rules under [cash-producing structures](/systems/produce-cash/#capture).
- The repair flag is cleared. A cloak generator's radius is reset and re-enabled if it still has power. EVA announces the capture when either house is under player control.
- The losing house records the structure as its recapture target. [Computer-controlled engineers](#computer-controlled-engineers) covers when its engineers head for it.

:::caution[A computer house sells what it captures]
Outside a campaign game, a computer house that captures a structure sells it. It first refunds and removes every upgrade plugged into it. A structure that produces buildings is kept when it is the only one of its type the capturing house owns.

A type with no build-up animation cannot be sold. Its upgrades are still removed and refunded, but the bare structure stays. A firestorm wall section is the exception: it is removed outright with no refund.
:::

A captured structure keeps three effects for the rest of the match:

- It produces fewer survivors when destroyed. The count is halved before the one-to-five limit, and each footprint cell has worse odds of producing one. [Survivors](#survivors) gives the numbers.
- It never produces the `[General]` [`Engineer`](/keys/engineer/#scope-global-rules) type as a survivor, whether it is destroyed or sold. An uncaptured structure that produces buildings can.
- It passes [the computer's last repair check](/systems/repair/#when-the-computer-repairs) without being marked for repair.

Nothing clears the captured state, so capturing the structure back undoes none of these.

Rank, abilities and the elite weapon [are kept](/systems/veterancy/#carrying-rank-between-objects). Strength does not change. An armed demolition charge keeps counting down.

## Demolition charges

### Arming a structure

A soldier arms a charge when it reaches a structure while on the sabotage mission, with that structure as its destination. The structure must still be `Repairable=yes` when the soldier arrives. If it is not, the soldier drops its target and destination and goes idle. There is no alliance test, so force fire can send a saboteur against a structure of its own house.

The charge is armed unless the structure is already being sold. The structure then flashes as a target, and the countdown starts at [`C4Delay`](/keys/c4delay/) minutes converted to frames. For example, `C4Delay=0.03` gives 27 frames. The saboteur is recorded against the structure.

Whether or not the charge was armed, the saboteur then drops its destination, uncloaks, waits out its rearm delay before it can fire again, and runs away from the structure.

### Detonation

When the countdown reaches zero, the structure takes damage equal to its current strength. The damage is forced, uses `C4Warhead`, and credits the saboteur. If the saboteur has died, the damage credits nobody, so its house gets no credit for the kill.

:::caution[A charge cannot be defused]
Nothing removes an armed charge. Repairing the structure, restoring it with an engineer and capturing it all leave the countdown running.

An armed charge blocks every sale except a player's sell order. A sale by a computer house, by a trigger action, or by the capture sell-off above is refused. A player's sell order starts selling the structure with the countdown still running.
:::

### Survivors

A destroyed structure produces up to `Cost × SurvivorRate / SurvivorDivisor` survivors, limited to between 1 and 5. The divisor is doubled for a captured structure. A type that is not [`Crewed=yes`](/keys/crewed/) produces none, and so does any structure when `SurvivorDivisor` is `0`.

Each footprint cell then rolls once for one survivor until that count is reached. Two facts set the odds: whether the structure was ever captured, and whether a live saboteur is on record against it. Arming a charge puts the saboteur on record, and the saboteur's death removes it.

| Roll per footprint cell | No saboteur on record | Saboteur on record |
| --- | --- | --- |
| Never captured | One in three | One in two |
| Captured | One in nine | One in eight |

A captured structure with a saboteur on record still rolls worse (one in eight) than an uncaptured one without (one in three).

If the saboteur on record is not allied to the structure's owner, the survivors attack it. Otherwise a computer house's survivors go hunting, and a human house's survivors only scatter.

A structure destroyed by its charge produces no survivors, because forced damage that destroys a structure leaves no survivors. The saboteur column therefore applies only when something else destroys the structure first, while the saboteur is still alive.

## Stealing a vehicle

`VehicleThief=yes` and `Thief=yes` are separate settings with separate cursors, targeting and limits. Both record the soldier type that took the vehicle. When a vehicle stolen either way is destroyed, a soldier of that type steps back out of the wreck. It is created at the wreck with strength between 5 and half its maximum, whatever `Crewed=` says and with no crew-escape roll. It hunts for a computer house and stands guard for a human one. A stolen aircraft does not return its thief.

A `VehicleThief=yes` soldier is the one with a cursor. It takes the vehicle through [the vehicle branch](#the-vehicle-branch).

A hunting vehicle thief turns its target into a capture order. An idle computer-owned one outside a team goes on area guard once its house's IQ reaches [`GuardArea`](/keys/guardarea/).

While the house owns a vehicle stolen by a `VehicleThief=yes` type, that vehicle counts against the type's positive [`BuildLimit`](/keys/buildlimit/). A hijacker limited to one cannot be rebuilt while its stolen vehicle survives. A stolen aircraft does not count.

When the vehicle a thief is targeting deploys into a mobile war factory or a construction yard, the thief drops its target. Every other object targeting that vehicle switches to the new structure.

When a crusher that is not `IsTrain=yes` drives over a crushable, non-allied vehicle thief heading for it, the thief is deleted and its house takes the crusher. The thief's tag does not move. This theft does not record the thief's type, so it counts against no build limit and no hijacker returns when the vehicle dies.

A `Thief=yes` soldier has no cursor. It tries to steal on every game frame, whatever its mission, whenever its movement destination is a non-allied vehicle. Aircraft and deployed vehicles do not count. Within half a cell and one height level, it takes the vehicle:

1. the vehicle's entered trigger springs;
2. the vehicle breaks radio contact;
3. everything tracking the vehicle drops it;
4. the thief's tag moves onto the vehicle, if it is transferable;
5. the vehicle changes owner, which springs its entered trigger again;
6. the thief is deleted.

Farther away, the thief re-aims at the vehicle whenever the vehicle moves off. The flag also [widens what its owner scans for](/systems/target-selection/#what-each-kind-of-object-considers).

## Computer-controlled engineers

A computer house aims engineers through [target selection](/systems/target-selection/#what-each-kind-of-object-considers). That page covers the scanning rules, the fifteen-cell shortcut to the house's recapture target, and the filter that accepts a damaged allied structure and rejects other allied ones. Four mission changes happen outside target selection:

- A hunt becomes a capture order for an engineer that has neither `C4=yes` nor the `C4` ability, and for a vehicle thief.
- A hunt becomes a sabotage order for a soldier with `C4=yes` or the `C4` ability whose target is a `Repairable=yes` structure.
- For a computer house only, guard and area guard become a sabotage order under the same condition.
- A patrolling engineer drops an allied structure as a target once that structure's strength rises above `ConditionRed`.

An engineer does not scan for targets on plain guard. A computer-owned engineer that takes damage on guard or area guard switches to hunt.

The guard and area guard changes keep the soldier's current destination. A computer saboteur whose destination is a cell can therefore run the sabotage mission and reach that cell. It then sets off three explosions centered on itself, each dealing [`BridgeStrength`](/keys/bridgestrength/) damage with `C4Warhead`. No player order produces this, because no cursor gives the sabotage order over a cell.

## Settings without effect

[`EngineerDamage`](/keys/engineerdamage/) is read from `[General]`, but nothing in play uses the value, so setting it changes nothing.
