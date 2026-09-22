---
title: Repair and healing
summary: "Restores structures for credits, services vehicles at a depot, heals infantry at a hospital, and mends objects that repair themselves."
category: buildings-economy
keys:
  - Ammo
  - Armory
  - ConditionRed
  - ConditionYellow
  - Cost
  - CreditReserve
  - EliteAbilities
  - Hospital
  - IQ
  - IRepairRate
  - IRepairStep
  - ManualReload
  - Mechanic
  - OmniHealer
  - ReloadRate
  - RepairBay
  - RepairDelay
  - RepairPercent
  - RepairRate
  - RepairSell
  - RepairStep
  - Repairable
  - SelfHealCap
  - SelfHealRate
  - SelfHealStep
  - SelfHealing
  - SelfHealingCap
  - SelfHealingRate
  - SelfHealingStep
  - SellBack
  - Strength
  - TiberiumHeal
  - URepairRate
  - UnitReload
  - UnitRepair
  - VeteranAbilities
related:
  - type: system
    id: ai-base-building
  - type: system
    id: veterancy
  - type: internal
    id: radio
  - type: command
    id: ToggleRepair
---

The game restores strength in five ways: the repair wrench on a structure, a service depot for vehicles and aircraft, a hospital for infantry, self-healing, and Tiberium healing. An armory uses the hospital's settings to promote infantry instead of healing them.

The paths share their rules settings, and a setting's name does not say which paths read it. Most settings reach more than one path. Changing `RepairRate` for the wrench also changes self-healing, and changing `IRepairRate` for the hospital also changes the armory. Only self-healing can be moved off the shared settings, using the three keys at the end of the table.

| Setting | What it controls |
| --- | --- |
| [`RepairRate`](/keys/repairrate/) | The interval between structure repair steps, and between self-healing steps when `SelfHealRate` is not set |
| [`URepairRate`](/keys/urepairrate/) | The interval between service depot steps. It sets no step size |
| [`IRepairRate`](/keys/irepairrate/) | The count a hospital reaches before it heals a step, and the count an armory reaches before it promotes |
| [`RepairStep`](/keys/repairstep/) | The strength one step restores to a structure, vehicle or aircraft, and a divisor in the credit cost |
| [`IRepairStep`](/keys/irepairstep/) | The strength one step restores to infantry |
| [`RepairPercent`](/keys/repairpercent/) | The multiplier at the end of the credit cost |
| [`TiberiumHeal`](/keys/tiberiumheal/#scope-global-rules) | The interval between Tiberium healing steps |
| [`SelfHealRate`](/keys/selfhealrate/) | The interval between self-healing steps, replacing `RepairRate` for self-healing only |
| [`SelfHealStep`](/keys/selfhealstep/) | The strength one self-healing step restores |
| [`SelfHealCap`](/keys/selfhealcap/) | The share of maximum strength at which self-healing stops, replacing `ConditionYellow` for self-healing only |

Each interval setting is a fraction of a minute, which the engine multiplies by 900 frames. The wrench, self-healing and Tiberium healing step on frames that are multiples of the result, truncated to whole frames. The depot, the hospital and the armory count up and act when the count reaches the result, rounded up to a whole number. At the engine defaults, with `SelfHealRate` unset, the paths step at these intervals:

| Path | Default interval |
| --- | --- |
| Structure repair | 14 frames |
| Self-healing | 14 frames |
| Tiberium healing | 15 frames |
| Service depot | 15 frames |
| Hospital | 15 frames |
| Armory | About 210 frames, explained under [Hospitals and armories](#hospitals-and-armories) |

## Repairing a structure

### Turning the wrench on

[Repair Mode](/commands/togglerepair/) does nothing while a building is waiting to be placed. It turns on only if the player owns at least one building, and turning it on deselects everything. Every change of the mode turns off sell mode, power mode and waypoint mode.

In repair mode, the wrench cursor appears only over a repair candidate that the player controls. Everything else shows the refusal cursor.

A building is a repair candidate under **all of**, in this order:

- its strength is above zero;
- it is not a deployed vehicle (a type with [`UndeploysInto`](/keys/undeploysinto/) counts as one unless it is a construction yard);
- **Any of:**
  - **All of:** its type is [`Repairable=yes`](/keys/repairable/), and it is below its maximum [`Strength`](/keys/strength/#scope-aircrafttype);
  - it carries a limpet mine, the mark a [`LimpetFactor`](/keys/limpetfactor/) warhead leaves on whatever it hits.

A mined structure is therefore a candidate even when it is undamaged.

Vehicles, infantry and aircraft are never candidates, so the wrench never appears over them. A damaged vehicle is served by [a depot](#service-depots) instead.

Each click switches the building's repair on or off. Switching it on plays the click sound and flashes the building. Switching it on at maximum strength plays the scold sound instead, but still switches repair on; only a mined structure can be in that state.

### The repair tick

A structure under repair takes a step on every frame that is a multiple of `RepairRate * 900`. Every structure repair in the match therefore steps on the same frames, every 14 frames at the default.

Each step charges the owner before it heals. If the owner's available money covers [the cost of one step](#the-cost-of-one-step), the money is spent and `RepairStep` strength is added. A step that reaches maximum strength caps the strength there and ends the repair.

After each paid step, the structure's damaged animations and damage smoke are updated. A structure that rises above [`ConditionYellow`](/keys/conditionyellow/) loses its damage smoke and returns to its undamaged artwork on that step.

### The cost of one step

```text title="cost of one repair step"
cost = (raw cost / (Strength / RepairStep)) * RepairPercent, never below 1
```

Both divisions drop the remainder, and the multiplication is truncated to whole credits. A full repair therefore rarely costs exactly `RepairPercent` of the building's price. Take a structure with a raw cost of 1000 and `Strength=400`, at the engine defaults of `RepairStep=5` and `RepairPercent=0.25`:

1. `400 / 5` gives 80 steps.
2. `1000 / 80` is 12.5, which the division cuts to 12.
3. `12 * 0.25` gives 3 credits per step.

Repairing the structure from one hit point takes 80 steps and costs 240 credits, not the 250 that the multiplier suggests.

The one-credit minimum raises the cost of a cheap structure with high strength. A structure with a raw cost of 100 and `Strength=1000` has 200 steps. `100 / 200` is already 0 before the multiplier applies, so every step costs the 1-credit minimum. A full repair costs 200 credits, twice what the structure cost to build.

The raw cost is the type's [`Cost`](/keys/cost/#scope-aircrafttype), before any country or difficulty multiplier. For a structure, the price of what it gives away is subtracted:

- the cost of its [`FreeUnit`](/keys/freeunit/);
- the average cost of the [`PadAircraft`](/keys/padaircraft/) types, on one structure type only: the first entry in the `Dock` list of the first `PadAircraft` type. This subtraction does not apply when [`SeparateAircraft=yes`](/keys/separateaircraft/), or when that structure's `FreeUnit` is an aircraft.

When the type has a `FreeUnit`, the result is never below 0. A refinery that comes with a free harvester is therefore cheaper to repair than its listed price suggests.

:::danger[Keep `RepairStep` and `RepairRate` in range]
Keep `RepairStep` above `0` and no larger than the `Strength` of any structure, vehicle or aircraft that can be repaired. A value of `0`, or one above such an object's `Strength`, crashes the game on the first repair step of that object, under the wrench or at a depot. So does a negative value further below `0` than that `Strength`.

A negative `RepairStep` closer to `0` does not crash. Each wrench step then takes that much strength from the structure, while a depot step restores `1` point.

Keep `RepairRate` at `1/900` (about `0.0011`) or above. A smaller value crashes the game on the first frame a structure is under repair. Self-healing also reads `RepairRate`, but it treats a too-small interval as one frame and does not crash.
:::

### What stops a repair

- Reaching maximum strength.
- A step the owner cannot pay for. The repair ends on that step.
- Another click.
- Selling the structure.
- Capture. The new owner receives the structure with repair off.
- An engineer. It restores the structure to maximum strength at no cost and switches repair off.

A limpet mine does not stop a repair, and repairing does not remove it. A mined structure at maximum strength therefore stays a candidate. If the player switches repair on, the next step charges one step's cost, caps the strength at the maximum again, and ends the repair. The mine stays.

## Service depots

### Two settings, two different questions

[`UnitRepair=yes`](/keys/unitrepair/) makes a building a service depot. The depot accepts vehicles and aircraft that ask to dock, offers the player's selected vehicles a move cursor onto its pad, and runs the repair cycle.

[`RepairBay`](/keys/repairbay/) names the one BuildingType that repair orders look for. The engine reads the two settings separately, so they can name different buildings:

- A `RepairBay` type without `UnitRepair=yes` refuses every docking request. A vehicle ordered to repair keeps searching for as long as its house owns one.
- A depot that `RepairBay` does not name serves anything that reaches it, but a repair order never sends a vehicle there.

:::danger[Set `RepairBay` to a building type]
If the rules name no `RepairBay` type, the game crashes when any of these happens:

- a vehicle receives the repair order;
- a harvester or weeder runs out of places to harvest, whichever house owns it;
- a computer-owned aircraft at or below [`ConditionYellow`](/keys/conditionyellow/) looks for a repair bay while its house has at least 100 credits.
:::

### Reaching the pad

Five routes lead to a depot. Some look for a `UnitRepair` building and some look for the `RepairBay` type.

| Route | Looks for | Conditions |
| --- | --- | --- |
| Player moves a vehicle onto a depot | `UnitRepair` | The depot is allied, not serving anyone, and holds nothing. The vehicle does not need to be damaged |
| Player sends an aircraft in | `UnitRepair` or a helipad | The building is allied, not serving anyone, and holds nothing |
| Repair order | `RepairBay` | The nearest own or allied building of exactly that type that accepts the vehicle, in the same [movement zone](/glossary/#movement-zone) as the vehicle's destination cell. The search runs only while the vehicle's house owns a building of that type |
| Computer vehicle, checked every 16 frames | `UnitRepair` | The vehicle is damaged, has no mission queued, and is not a harvester or weeder. It is on area guard duty, or on guard duty with no remembered position. It picks its house's nearest depot within twenty cell diagonals, a little over 28 cells |
| Computer aircraft | `RepairBay` | The aircraft is on guard duty below its flight level, and at or below [`ConditionYellow`](/keys/conditionyellow/). It is not in contact with another object, unless it has landed and that object is not a `RepairBay` building. Its house has at least 100 credits, a floor fixed in the engine. The building is chosen as for a repair order, except that aircraft skip the movement-zone test |

The repair order search normally takes the nearest building. A building with its house's [primary-factory flag](/systems/production/#the-primary-factory) is the exception: it replaces every building checked before it, whatever the distance. A building checked after it still replaces it if that building is strictly nearer. The check order follows the engine's internal building list, which nothing on the map shows. A depot has the primary-factory flag only if its type also produces something.

A computer vehicle remembers where it stood before it leaves for the depot, and returns there after the repair.

### Docking

A depot accepts a [docking request](/internals/radio/) only from an allied vehicle or aircraft. It also refuses while any of these is true:

- it is switched off;
- it is under construction, being sold, or still playing its buildup;
- it is already in contact with another object;
- the requesting object is already standing on it.

When the object arrives, the depot starts its service mission and the object goes to sleep. Once the object is within 150 leptons of the depot's center, the depot switches off the object's locomotor and clears its destination. This holds the object still on the pad.

### One step at a time

The depot asks the parked object for one repair step at a time. The object refuses while it has a destination.

The first request comes as soon as the object is parked:

- An object that needs nothing is released at once.
- If the first step is paid for, the repair cycle starts.
- If the owner cannot pay for the first step, the object stays on the pad. The depot asks again at each update of its mission, every 14 frames at the default [`Rate`](/keys/rate/#scope-mission-behavior) of the `[Repair]` mission, and makes no announcement.

During the cycle, the depot counts to `URepairRate * 900` before each further request, one count per frame. Steps therefore fall 15 frames apart at the engine default. Each step restores `RepairStep` strength. Its cost comes from [the structure formula](#the-cost-of-one-step), using the vehicle's or aircraft's `Cost` and `Strength`.

The depot acts on the object's answer to each request:

- **Paid.** The strength is added and the cycle continues.
- **Unaffordable.** The depot announces insufficient funds and stops the cycle. The object stays parked, and the depot asks again at each mission update as it does for the first step.
- **Anything else.** This covers a step that finishes the repair, an object that needs nothing, and an object that has been given a destination. The depot announces that the unit is repaired and releases the object. A computer-owned object with a remembered position returns there, as a computer vehicle does after [reaching the pad](#reaching-the-pad) on its own. Every other object moves to an exit cell beside the depot.

### What a depot does for free

Each request to an object with no destination clears its limpet mine and resets its turret and body turn rates to the type's [`ROT`](/keys/rot/#scope-aircrafttype), at no charge.

A [`ManualReload=yes`](/keys/manualreload/) object whose ammunition is not full is then refilled completely, also for free. The refill takes the place of the repair step for that request. An undamaged mine layer is therefore rearmed and released at once. A damaged one is rearmed at the first request and starts repairing at the next.

### Selling at the pad

Selling a depot while an object is parked within half a cell of it sells the object instead. The object is released and sold, the depot is not sold, and the depot returns to guard duty. The same condition is what lets the player sell a parked vehicle or aircraft at all.

A computer house sells a harvester or weeder that has run out of places to harvest, if it is below maximum strength. The depot first charges for and restores one step, then sells the harvester at the point where the repair cycle would start. At maximum strength, such a harvester is released at once like any object that needs nothing. A human house's harvester in the same state is repaired and released as usual.

### `UnitReload` is a different service

[`UnitReload=yes`](/keys/unitreload/) makes a building give the docked object one ammunition point per [`ReloadRate`](/keys/reloadrate/) interval, 45 frames at the engine default. It repairs nothing. A helipad needs this flag to rearm the aircraft that land on it.

A building runs only one service. Its service mission checks these flags in order and runs the first one the type sets:

1. construction yard;
2. `Hospital`;
3. `Armory`;
4. `UnitRepair`;
5. `UnitReload`.

The docking check uses a different order and tests `UnitRepair` before the two infantry flags. A type that is both a depot and a hospital therefore refuses infantry at the door.

## Hospitals and armories

[`Hospital=yes`](/keys/hospital/) and [`Armory=yes`](/keys/armory/) serve infantry only, and only infantry that the player sends in with the enter cursor. The building must be allied. A hospital offers the cursor to damaged infantry, and an armory to infantry that is not yet elite.

The building admits the infantry under **all of**, in this order:

- its house is allied with the infantry's;
- it is not under construction, being sold, or still playing its buildup;
- it is not in contact with anything other than this infantry;
- it is switched on;
- the caller is infantry, not a vehicle or an aircraft;
- its [`Ammo`](/keys/ammo/) count is not zero;
- it is not already serving someone.

Each admission uses up one point of `Ammo`.

:::caution[Give hospitals and armories an `Ammo` count]
Set `Ammo` to the number of visits the building should serve for the whole match. Hospitals and armories never restock, unlike other buildings. A building with no `Ammo` set admits one visitor and then refuses everyone for the rest of the match.
:::

A hospital heals [`IRepairStep`](/keys/irepairstep/) strength per step, at no charge. It takes a step each time its count reaches `IRepairRate * 900`, counting once per frame. It releases the occupant at maximum strength. An occupant that needs no healing is released at the first count and still uses up its admission.

An armory heals nothing. When its count reaches `IRepairRate * 900`, it promotes the occupant and releases it. Below-rookie infantry leaves as veteran, and any other infantry leaves as elite. A rookie therefore gains two ranks in one visit, and a veteran gains one. [Promotion without kills](/systems/veterancy/#promotion-without-kills) compares the armory with the other sources of rank. The [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/) of the new rank apply as soon as the occupant leaves.

:::caution[The same `IRepairRate` runs about fourteen times slower in an armory]
A hospital's count advances once per frame, so it reaches 15 in 15 frames at the default. An armory's count advances once per update of its service mission, every 14 frames at the default `Rate` of the `[Repair]` mission, so it takes about 210 frames. Changing `IRepairRate` changes both buildings by the same factor; neither can be tuned alone.
:::

## Self-healing

[`SelfHealing=yes`](/keys/selfhealing/), or the `SELF_HEAL` ability from [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/), makes an object heal itself with no building, order or credits. It works for structures, vehicles, aircraft and infantry.

Three settings decide what a tick does:

| Read from | Step | Interval | Ceiling |
| --- | --- | --- | --- |
| The object's type | [`SelfHealingStep`](/keys/selfhealingstep/) | [`SelfHealingRate`](/keys/selfhealingrate/) | [`SelfHealingCap`](/keys/selfhealingcap/) |
| The rules | [`SelfHealStep`](/keys/selfhealstep/) | [`SelfHealRate`](/keys/selfhealrate/) | [`SelfHealCap`](/keys/selfhealcap/) |
| Neither | `1` | [`RepairRate`](/keys/repairrate/) | [`ConditionYellow`](/keys/conditionyellow/) |

Read each column from the top. A value below zero passes to the row beneath. With none of them set, an object heals one point every 14 frames until it passes half strength. `RepairStep` and `IRepairStep` never affect self-healing.

Self-healing steps on frames that are multiples of the interval, so every object on the same interval heals on the same frames. Each tick adds the step and caps the result at maximum strength, so no setting overheals. A step below `1` heals `1` point, and an interval shorter than one frame heals on every frame.

A ceiling of `0` turns self-healing off. A tick needs the object's strength ratio to be at or below the ceiling. Any object with strength left has a ratio above `0`, so it never heals.

`SelfHealing=no` turns self-healing off unless the object has the `SELF_HEAL` ability.

An object at zero strength never heals. Only an aircraft can be at zero strength and still in play: one killed in the air keeps flying until it lands. Without this rule it could heal in mid-air and fly on.

:::caution[The ceiling is not the damage threshold]
Healing continues while the strength ratio is at or below the ceiling, so the last tick can carry the object up to one step past it. [`ConditionYellow`](/keys/conditionyellow/) still decides when an object counts as damaged, so a ceiling above or below it leaves healing and the damaged state out of step. When self-healing lifts a structure above `ConditionYellow`, its damage smoke stops. Its animations stay in their damaged form until something else refreshes them, such as a hit, a paid repair step, an engineer, an upgrade or the start of a new animation.
:::

Tiberium healing is a separate path. [`TiberiumHeal=yes`](/keys/tiberiumheal/#scope-aircrafttype) on an infantry, vehicle or aircraft type, or the `TIBERIUM_HEAL` ability, heals the object while its cell contains Tiberium. Height is not checked, so such an aircraft also heals while flying over Tiberium. Structures never heal this way.

The rules key of the same name, [`TiberiumHeal`](/keys/tiberiumheal/#scope-global-rules), sets the interval, 15 frames at the engine default. Each tick restores the type's repair step: `IRepairStep` for infantry and `RepairStep` for vehicles and aircraft, with a minimum of `1`. Tiberium healing continues to maximum strength and has no ceiling.

A weapon that deals negative damage, such as a medic's or a mechanic's, heals through ordinary damage and uses none of the settings on this page. Each heal also clears the target's limpet mine and resets its turn rates to the type's `ROT`.

A healer's kind decides what it can heal: infantry heal infantry, and every other healer heals vehicles, including landed aircraft. [`Mechanic=yes`](/keys/mechanic/) makes infantry heal vehicles instead of infantry, and [`OmniHealer=yes`](/keys/omnihealer/) makes a healer heal both.

## When the computer repairs

The computer decides on each of its buildings in turn. It switches a building's repair on when these tests pass, in this order:

1. The owning house's [`IQ`](/keys/iq/) is at or above [`RepairSell`](/keys/repairsell/), and the building is neither under construction nor being sold.
2. The building is a repair candidate by the same test the wrench uses.
3. The house's available money is at or above [`CreditReserve`](/keys/creditreserve/).
4. The house has not started a repair since its waiting time last ran out.
5. The building is not already under repair, and it has been captured, is flagged for repair, or belongs to a human-controlled house.

The repair then runs as an ordinary [repair tick](#the-repair-tick), paid from the house's money. Outside campaign games every computer house has the maximum IQ, so the first test passes whenever `RepairSell` is at or below [`MaxIQLevels`](/keys/maxiqlevels/).

Three things flag a building for repair:

- outside campaign games, placing any building of a computer house that is not [`MultiplayPassive=yes`](/keys/multiplaypassive/);
- a computer MCV deploying into a construction yard;
- a map record for the structure that flags it for repair.

After a computer house starts one repair, it waits a random time between `RepairDelay * 225` and `RepairDelay * 1800` frames before it may start another. At the [`RepairDelay`](/keys/repairdelay/) default that is 4 to 36 frames, so a base's repairs start one at a time. A human-controlled house has no waiting time and may start another repair on its next update.

:::caution[A map can give the player automatic repair and sale]
Human-controlled houses always pass the last test. A map that sets the player's house `IQ` at or above `RepairSell` therefore switches repair on for every damaged building that house owns, one after another, spending its credits without a click. The same setting lets the sale rules below sell that house's damaged buildings when its money falls under `CreditReserve`. An `IQ` above the maximum is replaced by `1`, so an oversized value turns this off unless `RepairSell` is `1` or lower.
:::

When the house's money is below its credit reserve, the computer considers selling the building instead. After the first two tests above, the sale needs **all of**, in this order:

- **Any of:** the game is not a campaign game, or the structure is marked sellable;
- the structure has been damaged by something its house is not allied with;
- the owning house's tech level is at or above [`SellBack`](/keys/sellback/);
- a random number from `0` to `50` is below that same tech level;
- the structure has no trigger tag;
- its type does not produce buildings;
- its strength ratio is below [`ConditionRed`](/keys/conditionred/).

The random number is drawn again on every frame while the other terms hold, so it delays a sale but does not prevent one.

The sellable mark belongs to the structure, not to its type's rules. A structure starts marked unless its type has no buildup animation. A structure placed by the map takes the mark from the field after the trigger name in its map record instead, whatever its type's artwork. A record that leaves the field out gives `0`, not sellable. Outside campaign games the mark is never checked.

The sale itself also requires a buildup animation, in every game mode. A type without one is never sold this way, whatever its mark.

A computer house repairs and sells its buildings only through this routine. [Rebuilding a destroyed structure](/systems/ai-base-building/) is a separate decision.
