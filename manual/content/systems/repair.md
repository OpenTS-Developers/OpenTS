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
  - ReloadRate
  - RepairBay
  - RepairDelay
  - RepairPercent
  - RepairRate
  - RepairSell
  - RepairStep
  - Repairable
  - SelfHealing
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

Five paths restore strength, and they share nothing but the settings they read: the wrench a player puts on a structure, the depot a vehicle drives onto, the hospital or armory an infantry walks into, the mending an object performs on itself, and the Tiberium a foot object stands in. The rate and step settings do not divide along those lines, and their names do not say which path each one reaches. The table gives each setting the paths it actually reaches; what to take from it is that most of them reach more than one, so retuning a setting for the wrench moves self-healing with it and retuning one for the hospital moves the armory.

| Setting | What it reaches |
| --- | --- |
| [`RepairRate`](/keys/repairrate/) | The interval between structure repair steps, and the interval between self-healing steps |
| [`URepairRate`](/keys/urepairrate/) | The interval between service-depot steps. It sets no step size |
| [`IRepairRate`](/keys/irepairrate/) | The count a hospital reaches before it heals a step, and the count an armory reaches before it promotes |
| [`RepairStep`](/keys/repairstep/) | The strength one step restores to a structure, vehicle or aircraft, and the divisor inside the credit cost |
| [`IRepairStep`](/keys/irepairstep/) | The strength one step restores to infantry |
| [`RepairPercent`](/keys/repairpercent/) | The multiplier at the end of the credit cost |
| [`TiberiumHeal`](/keys/tiberiumheal/#scope-global-rules) | The interval between Tiberium healing steps |

Each of the four intervals is a fraction of a minute, multiplied by 900 frames where it is used. At the engine defaults a structure repair step and a self-healing step fall every 14 frames, a Tiberium healing step every 15, and the counted paths — the depot, the hospital and the armory — act on the count of 15. What that count is worth in wall time differs by a factor of fourteen between the hospital and the armory, for the reason given under [Hospitals and armories](#hospitals-and-armories).

## Repairing a structure

### Turning the wrench on

[Repair Mode](/commands/togglerepair/) does nothing while a building is waiting to be placed, and it engages only if the player owns at least one building; every change of the mode clears sell mode, power mode and waypoint mode. Inside repair mode the cursor accepts an object only when its owner is under player control and the object is a repair candidate; everything else takes the refusal cursor.

A building is a candidate under **all of**, in this order:

- its strength is above zero;
- it is not a deployed vehicle — a type carrying [`UndeploysInto`](/keys/undeploysinto/) counts as one unless it is a construction yard;
- **Any of:**
  - **All of:** its type is [`Repairable=yes`](/keys/repairable/), and it stands below its maximum [`Strength`](/keys/strength/#scope-aircrafttype);
  - it carries a limpet mine, the mark a [`LimpetFactor`](/keys/limpetfactor/) warhead leaves on whatever it hits.

That last term is why a mined structure is a candidate at any strength, undamaged or not.

Non-buildings are rejected before any of those tests, so the wrench never appears over a vehicle, an infantry or an aircraft, and the click that would queue the order is issued for buildings alone. A damaged vehicle is served by [a depot](#service-depots) instead.

Each click toggles the flag. Turning it on below maximum strength plays the click sound and marks the building; turning it on at maximum strength plays the scold sound and still leaves the flag set — a state only a mined structure reaches.

### The repair tick

A repairing structure steps whenever the global frame counter is divisible by `RepairRate * 900`, so every repair running anywhere in the match lands on the same 14-frame boundary rather than on a boundary of its own.

One step charges the house before it heals: the cost is computed, and if the house's available money covers it the money is spent and the step is added to the structure's strength. Reaching maximum strength clamps the value and clears the flag. The animation damage state, the damage particle system and the idle animations are re-evaluated on every step that is paid for, so a structure crossing back above [`ConditionYellow`](/keys/conditionyellow/) drops its damage smoke and returns to undamaged artwork at that moment.

### The cost of one step

```text title="cost of one repair step"
cost = (raw cost / (Strength / RepairStep)) * RepairPercent, never below 1
```

Both divisions are integer divisions and the multiplication is truncated, so the credits charged over a full repair rarely land on `RepairPercent` of the building's price. Take a structure costing 1000 with `Strength=400`, at the engine defaults of `RepairStep=5` and `RepairPercent=0.25`. The step count is 400 divided by 5, or 80. One step costs 1000 divided by 80, which the integer division cuts from 12.5 to 12, multiplied by 0.25 and truncated again, giving 3. Eighty steps at 3 credits is 240 credits to rebuild the structure from one hit point, not the 250 the multiplier suggests.

The one-credit floor pushes the other way on anything cheap with a large strength. A structure costing 100 with `Strength=1000` has 200 steps, and 100 divided by 200 is already 0 before the multiplier runs, so every one of those steps pays the floor instead: 200 credits, twice what the structure cost to build.

The raw cost is the building's own [`Cost`](/keys/cost/#scope-aircrafttype) less whatever the structure hands out — the cost of its [`FreeUnit`](/keys/freeunit/), and, on the structure that the first entry of [`PadAircraft`](/keys/padaircraft/) docks at, the average cost of the first two pad aircraft unless [`SeparateAircraft=yes`](/keys/separateaircraft/). A construction yard that comes with a free vehicle is therefore cheaper to repair than its listed price implies.

:::danger[Two settings can divide by zero]
`Strength / RepairStep` is evaluated first and in integers: a `RepairStep` of `0` divides by zero outright, and one larger than the type's `Strength` makes that term zero so the next division crashes the game. `RepairRate * 900` is truncated to an integer and used as a modulus, so any value between zero and `1/900` crashes it as well — and because self-healing uses the same modulus, that second crash reaches every object with `SelfHealing=yes`, not only structures.
:::

### What stops a repair

- Reaching maximum strength.
- A step the house cannot pay for. The flag is cleared silently at that tick; unlike the depot, the wrench reports nothing.
- Another click.
- Selling. Deconstruction forces the flag off as its first act.
- Capture, which clears the flag for the new owner.
- An engineer, which restores a building to maximum strength outright and forces the flag off. That restoration is not a repair step and costs nothing.

A limpet mine is not one of them. The mark survives the repair, so a mined structure at maximum strength stays a candidate: the wrench engages, one step's credits are spent for nothing at the next tick, and the repair flag clears on that same tick, when the step's strength is clamped back down to the maximum. Nothing in that sequence touches the mark itself.

## Service depots

### Two settings, two different questions

[`UnitRepair=yes`](/keys/unitrepair/) makes a building answer the docking request, offer the player's move-onto-the-pad cursor, and run the repair cycle. [`RepairBay`](/keys/repairbay/) names the single BuildingType that a repair order steers toward. They are read independently and can name different things.

A `RepairBay` type without `UnitRepair=yes` refuses every docking request, and a vehicle ordered to repair keeps retrying the search for as long as its house owns one of them. A depot that `RepairBay` does not name still serves anything that reaches it, but no repair order will ever send a vehicle there.

:::danger[`RepairBay` must name a building type]
With no `RepairBay=` in the rules the setting names no building type at all, and neither the docking-bay search nor the ownership count behind it checks that before using the name. Any vehicle that takes the repair order crashes the game, including a computer harvester that has run out of Tiberium and heads for the pad to be sold.
:::

### Reaching the pad

Five routes end on a pad, and they do not agree about which of the two settings decides that a building will do.

| Route | Keys on | Conditions |
| --- | --- | --- |
| Player moves a vehicle onto a depot | `UnitRepair` | The building is allied, idle, and holds nothing; damage is not required |
| Player sends an aircraft in | `UnitRepair` or a helipad | The building is idle and holds nothing |
| Repair order | `RepairBay` | The nearest building of exactly that type that answers, own house or allied, in the same [movement zone](/glossary/#movement-zone) as the cell the vehicle is heading for. The scan runs at all only while the vehicle's own house owns one of the type. A building carrying its house's [primary-factory flag](/systems/production/#the-primary-factory) is taken whatever its distance, and its own distance then becomes the figure to beat, so it displaces every building scanned before it and is displaced in turn by any building scanned after it that is strictly nearer. Which of them the scan reaches first follows the order the engine holds its buildings in, which nothing on the map shows. A depot carries that flag only if its type also produces something |
| Computer vehicle, every 16 frames | `UnitRepair` | Own house only, below maximum strength, on guard or guard-area duty, not a harvester or weeder, within twenty cell diagonals of the building — a little over 28 cells |
| Computer aircraft | `RepairBay` | At or below [`ConditionYellow`](/keys/conditionyellow/) with at least 100 credits in hand, a floor fixed in the engine |

A computer vehicle records where it was standing before it leaves, and is sent back there once the repair finishes.

### Docking

The building answers a [docking request](/internals/radio/) only for an allied object, only while it is switched on and neither under construction nor being deconstructed, and only while it is not already in contact with someone else. A depot accepts a vehicle or an aircraft, and refuses one that is already standing on it. On arrival the building takes the repair mission and the client is put to sleep.

The building then pins its customer: once the client is within 150 leptons its locomotor is powered off and its destination is cleared, which is what keeps a vehicle still on the pad. The power-on half of that handling is suppressed while an ion storm is running, so a depot does not restore a client's power for as long as the storm lasts.

### One step at a time

Every exchange is a request the building makes and the client answers, and the client answers only while it is standing still. The first request comes as soon as the customer is parked: a client that needs nothing is released again straight away, and the repair cycle opens only once a step has actually been paid for. A client whose house cannot afford that first step stays on the pad and is asked again on every mission update — 14 frames at the default of the mission's own [`Rate`](/keys/rate/#scope-mission-behavior) — without any announcement.

Inside the cycle the depot counts to `URepairRate * 900` before each further request. The count advances once per frame, so steps fall about a second apart at the engine defaults. A step at the depot is the same arithmetic as a structure's — the same [`RepairStep`](/keys/repairstep/) and the same cost formula, for vehicles and aircraft alike, since neither type overrides those figures — and the depot acts on the answer rather than on its own view of the client:

- **Paid.** The strength is added and the cycle continues.
- **Unaffordable.** The depot announces the shortfall and drops back to idle with the client still parked.
- **Anything else.** A finished repair, a client that needs nothing, and a client that has been given somewhere to go all end the visit the same way: the depot announces a completed repair and releases the client to the position a computer house archived for it, or to an exit cell beside the building.

### What a depot does for free

Every request first clears the client's limpet mark and resets its turret and body rates of turn to the type's [`ROT`](/keys/rot/#scope-aircrafttype), at no charge.

A [`ManualReload=yes`](/keys/manualreload/) client whose magazine is not full then has it filled in one go, also free, and that refill pre-empts the repair for that exchange. An undamaged mine layer is therefore rearmed and released on the spot, and a damaged one spends its first exchange rearming and begins repairing at the next.

### Selling at the pad

Selling a depot that has something parked within half a cell sells the parked object instead: the customer is released from radio contact, sold, and the depot returns to guard duty untouched. The same proximity makes a docked vehicle or aircraft a legal sell target for the player in the first place.

A computer house sells a client outright at the moment the repair cycle would otherwise open, when the client has nothing left to do — the state a harvester reaches with no Tiberium in range. A human house's harvester in the same state is repaired and released instead.

### `UnitReload` is a different service

[`UnitReload=yes`](/keys/unitreload/) is a separate branch that hands the docked object one ammunition point per [`ReloadRate`](/keys/reloadrate/) interval, 45 frames at the engine default, and repairs nothing. A helipad needs the flag to rearm the aircraft that land on it.

Only one branch runs per building. The mission checks construction yard, then hospital, then armory, then `UnitRepair`, then `UnitReload`, and stops at the first flag the type carries. The docking answer is ordered differently, testing `UnitRepair` before the two infantry flags, so a type that is both a depot and a hospital turns infantry away at the door.

## Hospitals and armories

[`Hospital=yes`](/keys/hospital/) and [`Armory=yes`](/keys/armory/) take infantry only, and only through the enter cursor of an object under player control. A hospital offers the cursor while the infantry stands below its maximum strength, and an armory offers it while the infantry is not already elite; both need the building to be allied.

The building's own answer to the request needs **all of**, in this order:

- its house is allied with the infantry's;
- it is neither under construction nor being deconstructed, and is not still playing its buildup;
- it is not in radio contact with anything other than this infantry;
- it is switched on;
- the caller is infantry rather than a vehicle or an aircraft;
- its [`Ammo`](/keys/ammo/) pool is not zero;
- it is not already running the service mission for somebody else.

Admission then spends one point of that pool.

:::danger[A building with no `Ammo` serves exactly one visitor]
An unset ammunition pool is `-1`, which passes the non-zero test. The decrement on the first admission takes it to `-2` and the clamp that follows pins it at `0`, and the instant restock that refills every other building's ammunition explicitly skips hospitals and armories. The building then refuses everyone for the rest of the match. Give a hospital or an armory an explicit `Ammo` count equal to the number of visits it should ever serve.
:::

A hospital heals on the infantry figures: [`IRepairStep`](/keys/irepairstep/) strength per step and no charge at all, because infantry replace the credit formula with zero rather than paying what a vehicle would. Each count of `IRepairRate * 900` buys one step, the count advances once per frame, and the occupant is released once it reaches maximum strength. An occupant that turns out to need nothing is released at the first count, having still spent the admission point.

An armory heals nothing at all. When its count elapses it promotes the occupant and shows it the door: a below-rookie infantry leaves veteran and everything else leaves elite, so a rookie gains two ranks in one visit and a veteran gains one. [Promotion without kills](/systems/veterancy/#promotion-without-kills) places the armory among the other sources of rank, and the [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/) the new rank unlocks apply from the moment the occupant walks out.

:::caution[The same setting runs fourteen times slower in an armory]
A hospital's loop asks to be called again on the next frame, so its count advances once per frame and completes in about 15 frames. The armory's loop asks for nothing and falls back to the delay its mission carries — 14 frames at the default of the mission's own [`Rate`](/keys/rate/#scope-mission-behavior) — so its count advances once per invocation and takes about 210 frames to reach the same figure. `IRepairRate` cannot be tuned for one building kind without moving the other by the same factor.
:::

## Self-healing

[`SelfHealing=yes`](/keys/selfhealing/), or the `SELF_HEAL` ability from [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/), makes an object mend itself with no building, no order and no credits. The tick is the same `RepairRate` modulus the wrench uses, on the same global frames, and it applies to structures, vehicles, aircraft and infantry alike.

The amount is one strength point per tick. Nothing scales it: `RepairStep` and `IRepairStep` are not consulted on this path.

:::caution[Self-healing stops at the yellow line]
The tick is refused as soon as the object's strength ratio rises above [`ConditionYellow`](/keys/conditionyellow/), so healing ends one point past that threshold. At the engine default a self-healing object recovers to just over half strength and stays there; only raising `ConditionYellow` raises where it stops.
:::

Tiberium healing is the contrasting case. [`TiberiumHeal=yes`](/keys/tiberiumheal/#scope-aircrafttype), or the `TIBERIUM_HEAL` ability, restores a foot object standing on Tiberium every `TiberiumHeal * 900` frames — 15 at the engine default — and the amount is the type's repair step, `IRepairStep` for infantry and `RepairStep` for everything else. It runs while the object is below maximum strength and clamps to that maximum, so unlike self-healing it finishes the job. Buildings never heal this way.

A weapon that deals negative damage, as a medic or a mechanic does, restores strength through ordinary combat processing rather than through any path on this page. It also clears the target's limpet mark and resets its rates of turn to the type's `ROT`.

## When the computer repairs

The computer's decision runs on each of its buildings, in this order:

1. The owning house's [`IQ`](/keys/iq/) is at or above [`RepairSell`](/keys/repairsell/), and the building is neither under construction nor being deconstructed.
2. The building is a repair candidate by the same test the wrench uses.
3. The house's available money is at or above [`CreditReserve`](/keys/creditreserve/).
4. The house has not started a repair since its throttle last expired.
5. The building is not already repairing, and is either captured, flagged for repair, or owned by a human-controlled house.

Only then is the wrench switched on; from there the repair is the ordinary tick, paid for out of the same treasury. Outside campaign games every computer house is forced to the maximum IQ, so the first gate filters nothing there as long as `RepairSell` stays at or below [`MaxIQLevels`](/keys/maxiqlevels/).

The repair flag is set on every building of a non-human, non-passive house at the moment it appears outside a campaign game, on the construction yard a computer MCV deploys, and on any structure a map's own building line marks for repair.

After a house starts one repair it waits between `RepairDelay * 225` and `RepairDelay * 1800` frames — 4 to 36 frames at the [`RepairDelay`](/keys/repairdelay/) default — before it may start another, which spreads a base's repairs out instead of committing the treasury to all of them at once. A human-controlled house arms no such timer, so its permission returns on the next house pass.

:::caution[A map can give the player automatic repair]
The human-player clause sits in the same list as the repair flag, so a map that sets `IQ` at or above `RepairSell` on the player's own house turns the wrench on automatically for every damaged building that house owns, spending its credits without a click. A house's `IQ` above the maximum is replaced by `1` rather than by the maximum, so an over-large value switches this off rather than on.
:::

When the house is below its credit reserve, the same routine considers selling the building instead. Past the first two gates above, that decision needs **all of**, in this order:

- **Any of:** the game is not a campaign game, or the structure is marked sellable;
- the structure has taken damage from something its house is not allied with;
- the owning house's tech level is at or above [`SellBack`](/keys/sellback/);
- a draw from `0` through `50` comes out below that same tech level;
- the structure carries no trigger tag;
- its type produces no buildings;
- its strength ratio is below [`ConditionRed`](/keys/conditionred/).

The mark in the first term is not a rules setting. Every structure starts marked, and loses the mark outright when its type has no build-up animation. A structure the scenario placed takes the mark from the field that follows the trigger name in its own record instead, whatever the type's artwork holds, and that field reads as `0` when the record leaves it out. Outside a campaign game the term is skipped and the mark is never consulted at all.

The build-up animation is tested again by the sale itself, in every mode, so the fallback is a no-op for a type that has none however the mark stands.

Neither branch consults the house's base layout: a computer house repairs and sells through this routine alone, and [rebuilding a destroyed structure](/systems/ai-base-building/) is a separate decision.
