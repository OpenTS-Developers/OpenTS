---
title: Production and factories
summary: "Builds one object of each kind per house at a time, charging for it step by step, and hands the finished object to a factory building to leave from."
category: buildings-economy
keys:
  - Armory
  - BuildLimit
  - BuildSpeed
  - BuildTime
  - BuildupTime
  - ConstructionYard
  - Cost
  - DoubleOwned
  - Factory
  - FreeUnit
  - GameSpeedBias
  - Gate
  - GateStages
  - Hospital
  - MaximumQueuedObjects
  - MinProductionSpeed
  - MultipleFactory
  - Owner
  - PadAircraft
  - PlacementDelay
  - Prerequisite
  - PrerequisiteBarracks
  - PrerequisiteFactory
  - PrerequisiteGDIFactory
  - PrerequisiteNodFactory
  - PrerequisitePower
  - PrerequisiteRadar
  - PrerequisiteTech
  - ScoldSound
  - SeparateAircraft
  - TechLevel
  - Wall
  - WallBuildSpeedCoefficient
  - WeaponsFactory
related:
  - type: system
    id: ai-base-building
  - type: system
    id: power
---

Production runs on two models. A player's house has four production slots: one for infantry, one for vehicles, one for aircraft and one for structures. Each slot holds one object under construction, with a queue behind it, however many factories the house owns. Six war factories do not let such a house build six vehicles at once. They make the one vehicle it is building arrive sooner. A computer house does not use those slots at all. Each of its factory structures runs production of its own, so six war factories do turn out six vehicles at a time. All six are the same type, because the house names one type per category at a time and every idle factory takes it.

Both models put the object through 54 production steps and charge part of its price at every step. The finished object then waits out of play until a factory building lets it out.

## What counts as a factory

A BuildingType becomes a factory by naming the kind of object it produces.

```ini title="rules.ini"
[MYWEAP] ; example war factory BuildingType
Factory=UnitType
WeaponsFactory=yes
Owner=GDI,Nod
```

When a player's house orders an object, it searches its structures for one that clears all of these, in this order:

- it is on the map;
- it is owned by this house;
- its [`Factory=`](/keys/factory/) names the kind of object being ordered;
- it is switched on;
- it is neither being sold nor queued to be sold;
- the product is not blocked by its own [build limit](#build-limits);
- the factory type's [`Owner=`](/keys/owner/) shares at least one country with the product's `Owner=`;
- **Any of:** the factory's own type is not listed in [`BuildConst`](/keys/buildconst/), [`MultiMCV=yes`](/keys/multimcv/) is set, or that construction yard acts for a country and it is one of the product's owners.

That last term is why a captured construction yard keeps building its original owner's structures.

The first structure found with the primary flag is taken immediately; otherwise the last structure that passed is used. For an aircraft, a pad that already has an aircraft in radio contact with it is held back and used only when no free pad passed. A computer house runs no such search for its own production: its structures build whatever their house has decided to build next.

:::caution[The four object names split the two production models]
The value is matched against the engine's object-kind names without regard to case, and a name it does not recognize leaves the type a non-factory. Four of the names it does recognize (`Unit`, `Infantry`, `Aircraft` and `Building`) stand for objects on the map rather than for object types. The player's factory search tests the value against the product's own kind name, which is always the `...Type` form. A structure with one of the short names therefore never produces for a player. Even so, it still raises the multiple-factory divisor for its category. A computer house's building-driven production accepts the short names, so the same structure produces the computer's current choice normally. Write `UnitType`, `InfantryType`, `AircraftType` or `BuildingType` to get a factory that works for both.
:::

### The primary factory

One structure of each kind may be flagged as the primary factory. Because the search returns it as soon as it is seen, the primary is the structure a player's finished object comes out of. Toggling the flag clears it from every structure this house owns on the map whose `Factory=` names the same kind, then sets it on the chosen one. Toggling the current primary itself only clears it, leaving none. EVA announces the change to a player-controlled house. A structure loses the flag when it is captured, and an [`IsMobileWar=yes`](/keys/ismobilewar/) structure runs that toggle on itself as it opens.

The cursor that offers the toggle appears only over a structure clearing all of these, in this order:

- it is not stunned: only an [EM pulse](/systems/emp-pulse/) stuns a structure, holding it switched off and refusing to let it back on while it lasts;
- its `Factory=` names a kind of object;
- it belongs to the local player;
- its house owns more than one factory of that kind;
- for an infantry factory, some other structure of this house names `InfantryType` in its `Factory=`.

That last term reads the long name only. A second barracks written `Factory=Infantry` satisfies the count above it but not this term, so the cursor stays unoffered.

:::caution[The automatic primary flag does not count factories]
A factory is also flagged primary as it is placed, but the count is not of factories of its kind. It is the number of structures the house owns of one fixed BuildingType: the eighth entry of `[BuildingTypes]`, whatever that turns out to be. Which newly built factories come up primary therefore depends on the order of that list rather than on the base. Setting the primary deliberately is the toggle above.
:::

## What a house may build

Four gates stand between an object type and a house's build list, and they are tested in order.

### Tech level

[`TechLevel=-1`](/keys/techlevel/#scope-aircrafttype) blocks the type outright. Otherwise the type's level must not exceed the house's own. That level comes from [the house's own map section](/keys/techlevel/#scope-house-per-scenario) and defaults to the scenario number. Every house a non-campaign session sets up is instead given the level chosen for that session, seeded from [`[MultiplayerDefaults] TechLevel`](/keys/techlevel/#scope-global-rules).

### Prerequisites

Every entry of the type's [`Prerequisite=`](/keys/prerequisite/) list must be satisfied. An entry naming a BuildingType requires the house to own at least one live structure of exactly that type. The tally counts a structure from the moment it is placed, so a prerequisite unlocks while the structure is still playing its buildup animation. It also keeps counting a structure that has been switched off. Seven further entries are group names, each satisfied by owning anything on the matching rules list:

| `Prerequisite=` entry | List read |
| --- | --- |
| `POWER` | [`PrerequisitePower`](/keys/prerequisitepower/) |
| `FACTORY` | [`PrerequisiteFactory`](/keys/prerequisitefactory/) |
| `BARRACKS` | [`PrerequisiteBarracks`](/keys/prerequisitebarracks/) |
| `RADAR` | [`PrerequisiteRadar`](/keys/prerequisiteradar/) |
| `TECH` | [`PrerequisiteTech`](/keys/prerequisitetech/) |
| `GDIFACTORY` | [`PrerequisiteGDIFactory`](/keys/prerequisitegdifactory/) |
| `NODFACTORY` | [`PrerequisiteNodFactory`](/keys/prerequisitenodfactory/) |

:::caution[An upgrade prerequisite is answered by one structure only]
When a prerequisite names a type that plugs into another structure (one with [`PowersUpBuilding=`](/keys/powersupbuilding/)), the test does not scan the base for that plug. It takes a single structure: the newest one this house owns that is on the map, switched on and not being sold. It then asks whether that structure has the upgrade. The same plug installed anywhere else does not answer the prerequisite.
:::

### Ownership

For a BuildingType, its `Owner=` list must not be empty. The house must also own a construction yard that is on the map, switched on, not being sold, and acting as one of the countries in that list. A structure no yard can produce is not offered. [`MultiMCV=yes`](/keys/multimcv/) drops the yard test from both places. [`DoubleOwned=yes`](/keys/doubleowned/) opens the type to every country, but only outside campaign games. Units, infantry and aircraft are not put through this gate at all. Their ownership is enforced by the factory-and-product `Owner=` overlap above.

### Build-limit gate

The last gate, and the only one the factory search puts again on its own; [build limits](#build-limits) covers it.

### Computer houses

Only the first gate applies to a computer house. Once its tech level clears, every remaining test is skipped and the type counts as buildable; what it actually produces is decided by [base planning](/systems/ai-base-building/) instead.

## Build limits

[`BuildLimit=`](/keys/buildlimit/) decides which of two tallies it is compared against, and the sign is what selects them.

| `BuildLimit=` | Counted | Effect |
| --- | --- | --- |
| Above zero | Objects of the type the house owns now | A destroyed object frees its slot |
| Zero | — | The type can never be built |
| Below zero | Objects of the type the house has ever produced | Destroying one frees nothing |

Two adjustments apply to the "owns now" tally. A UnitType that names a [`DeploysInto=`](/keys/deploysinto/) structure also counts every structure of that type the house owns, so an MCV's limit is consumed by the construction yards it turned into. An InfantryType marked as a vehicle thief also counts every vehicle of this house that one of them is sitting in.

While a factory is already building that exact type, the answer is "buildable" rather than "at the limit", so the cameo does not vanish partway through a build. A type at a positive limit is drawn darkened on the sidebar rather than removed; one whose zero-or-negative limit is spent loses its cameo outright.

The limit reaches production through the player's path only, in two places. A new order stops at the factory search, which rejects a structure outright when the product is at its limit. The queue gate applies its own version, adding the object under construction and everything already queued to the tally. Its vehicle branch skips the deployed-building addition the factory search makes. That version has no case for structures at all, consistent with structures never being queued.

:::caution[Build limits do not restrain a computer house]
Neither test lies on the path a computer house's factories take, so the computer keeps producing the type past its limit. The limit does reach a computer house on a few side paths, but never its factories. The most visible side path is team creation, where a campaign team naming a build-limited member cannot be created. A `BuildLimit=` is therefore chiefly a limit on what the player may build.
:::

## How long it takes

Build time is computed in frames, in a fixed order, and truncated to a whole number at every step.

1. The object's [`Cost=`](/keys/cost/#scope-aircrafttype), multiplied by [`BuildSpeed`](/keys/buildspeed/) and by nine tenths of a frame per credit. A structure uses that figure as written, which already includes the price of any [`FreeUnit`](/keys/freeunit/) or pad aircraft bundled into it. The reduced figure that strips those out prices a repair step, not this one.
2. Multiplied by the figure the house was handed when it was given its difficulty. That figure multiplies together the country's [`BuildTime=`](/keys/buildtime/#scope-housetype), the difficulty setting's [`BuildTime=`](/keys/buildtime/#scope-difficulty-settings) and [`GameSpeedBias`](/keys/gamespeedbias/). A campaign game drops the country's figure.
3. Divided by the house's power multiplier.
4. Multiplied by the multiple-factory adjustment.
5. For a [`Wall=yes`](/keys/wall/#scope-buildingtype) BuildingType, multiplied by [`WallBuildSpeedCoefficient`](/keys/wallbuildspeedcoefficient/).

### Production steps

That figure is not the delay itself. It is divided by 54 and clamped between 1 and 255 to give the number of frames between production steps, and the object then takes 54 of those steps.

With every multiplier above at 1, a unit costing 1000 credits reaches that division with 900 frames. The truncation drops the remainder, giving 16 frames per step, so it finishes in 864 frames: 57.6 seconds rather than the full minute the price implies. The clamp fixes both ends of the range: nothing builds in less than 54 frames, or 3.6 seconds, which on those same multipliers covers everything costing under 120 credits. Nothing takes more than 13,770 frames, about 15.3 minutes, reached at 15,300 credits.

### More than one factory

The count of structures whose `Factory=` names the product's category adjusts the build time once through [`MultipleFactory`](/keys/multiplefactory/), rather than once per factory. Structures that are switched off or still in buildup count too. The multiplier is one divided by the product of `MultipleFactory` and one less than the count, so one factory leaves the time alone. The table gives the multiplier each count produces at the default setting and at half of it. The two figures worth taking from it are both on its second row: the second factory buys nothing at the default, and a `MultipleFactory` below 1 makes a pair of factories slower than a single one. Raising `MultipleFactory` above 1 is what makes a second factory pay off.

| Factories | Multiplier at `MultipleFactory=1` | At `MultipleFactory=0.5` |
| ---: | --- | --- |
| 1 | 1 | 1 |
| 2 | 1 | 2 |
| 3 | 0.5 | 1 |
| 4 | 0.333 | 0.667 |

A value of zero or below skips the adjustment entirely.

### Power

A house short of power divides its build time by a multiplier taken from a fixed ladder and floored at [`MinProductionSpeed`](/keys/minproductionspeed/). [The production ladder](/systems/power/#production) has the bands. The power recalculation runs whenever a structure is placed, lost, damaged or switched, balance change or not. It is the only thing that re-rates a house's factories, and re-rating keeps the step a build had already reached. Low power therefore slows a build in place instead of restarting it.

### Difficulty and campaign games

The country and difficulty multipliers are combined once, when the house is assigned its difficulty, and not per order. A campaign game drops the country's contribution from both the build-time and the price multiplier. A `[GDI]` or `[Nod]` section's `Cost=` and `BuildTime=` therefore shape skirmish and multiplayer games only. The two difficulty settings are separate axes. A difficulty block's `Cost=` changes what everything costs without changing how long it takes. Its `BuildTime=` changes how long it takes without changing what it costs.

## Paying for it

The full price is charged in installments across the 54 steps. The installment is recomputed at every step as the outstanding balance divided by the number of steps left, so integer division never loses or gains credits. Whatever remains at step 54 is charged in one final payment, and the balance reaches exactly zero.

An installment larger than the house's credits and stored tiberium put together stops the build. The step is rolled back, nothing is charged, and the same step is attempted again after another delay. Progress stays exactly where it stood until money arrives. Starting or resuming a build that the house cannot yet afford is allowed, and it sits at its first step on the same terms until the money is there.

Putting a build on hold moves no money and keeps the step it had reached. Canceling refunds the price less the outstanding balance, which is exactly what has been paid so far. It deletes the object under construction and frees the slot. Because the refund is worked out from the price at the moment of canceling, a price multiplier that changed mid-game skews it.

## The queue

Queues belong to a player's slots; a computer house's factory holds one object and nothing behind it. Ordering something while its slot is busy adds it to that slot's queue rather than starting it. The queue holds at most [`MaximumQueuedObjects`](/keys/maximumqueuedobjects/) entries, which puts a ceiling of one plus that number on how many objects of a kind can be outstanding. An order refused because the queue is full, or because the type is at its build limit, is dropped. For a player-controlled house it also plays [`ScoldSound`](/keys/scoldsound/).

A slot that has been put on hold queues new orders too, with one exception: ordering the very type that is sitting on hold resumes it instead of queuing a second copy. When the object in progress leaves, is canceled, or becomes unbuildable, the head of the queue is taken and started as though it had just been ordered.

Structures never queue. A second structure order is refused outright while any structure order is outstanding, whether the first is building, on hold, or finished and waiting to be placed. The [sidebar](/systems/sidebar/#clicking-a-build-cameo) turns the click away before it becomes an order. A held structure order can in principle be abandoned in favor of a new one, but nothing reaches that path. The sidebar is the only thing that raises a structure order in the first place. While a house has any structure on order, every structure cameo on its sidebar is drawn darkened.

## Leaving the factory

At step 54 the slot suspends itself and the object waits. How it gets out depends on who ordered it.

A player's completed vehicle, aircraft or infantry announces itself and asks its factory to let it out at once. A completed structure announces itself and waits to be placed by hand, which switches the display into placement mode. When no structure could let the object out, the request does nothing at all and the object keeps waiting. Clicking its cameo at that point cancels the order and announces that there is no factory.

The exit attempt itself reports success, a temporary blockage, or a permanent failure. Success hands the object to the house and clears the slot. On a player's path anything else cancels a vehicle, aircraft or infantry and refunds it, so a blockage the factory would have cleared costs the order. A structure is never canceled that way, and stays in the slot to be placed by hand.

A structure placed by hand is put down on the chosen cell, plays the placement sound and clears the cursor. When its type is a firestorm wall or lays a wall overlay, it also fills the gap between itself and a nearby wall of the same house. A cell that refuses the structure leaves it in the slot.

What letting the object out involves depends on the factory:

- An aircraft appears at its pad unless the pad already has an aircraft in contact with it. Otherwise it is spawned at the edge of the playable area and flies in. An ion storm places it on a nearby cell instead of either.
- A [`WeaponsFactory=yes`](/keys/weaponsfactory/) structure runs a door sequence and clears whatever is standing on its exit cell. While it is still unloading it hands the next object to a second idle structure of the same type, which lets the object out through its own exit. With no such structure free, the attempt is a temporary blockage.
- Every other factory picks an exit cell beside its footprint and sends the object there; finding no exit cell at all is a permanent failure. A barracks offsets the exit to a door coordinate of its own.
- One object leaves at a time: a factory still in contact with the object that just left refuses the next attempt as a temporary blockage. [`Hospital=yes`](/keys/hospital/), [`Armory=yes`](/keys/armory/) and `WeaponsFactory=yes` structures are exempt from that rule.
- A refinery or weeder puts a vehicle out one cell south-west of its center and one further south of that, then sends it to harvest. It reports a permanent failure anyway, so the vehicle does not stay: the order is canceled and refunded. Infantry leaving one of those is scattered and fails the same way.

A jumpjet infantryman whose rally point calls for flight skips the adjacent exit cell, flies straight to the rally point, and releases the factory's radio contact immediately. A nearby rally point, no rally point, and every other kind of infantry keep the ordinary exit-cell movement and unload coordination.

For a computer house, a vehicle produced by a factory with `WeaponsFactory=no` keeps that adjacent exit cell as its immediate destination. The base position chosen for it is queued behind the exit and becomes the point it guards. When the house supplies no valid position, the vehicle only clears the factory.

A computer house drives its factories from the structures themselves rather than from a sidebar, and treats the three outcomes differently. A temporary blockage arms a wait of [`PlacementDelay`](/keys/placementdelay/) minutes before the next attempt. Only a permanent failure abandons and refunds. A factory of its own that has stopped making progress with no wait armed is abandoned outright rather than left on hold. An idle one with more than 10 credits behind it asks its house what to build next and starts it.

### Buildup

A structure that has just been placed runs its construction animation before it opens for business. It tells whatever built it that construction has begun, which is what puts a [`ConstructionYard=yes`](/keys/constructionyard/) structure into its [`PreProductionAnim`](/keys/preproductionanim/). It tells the builder again when the animation ends. The buildup then stops and the new structure opens.

The step delay for that animation is [`BuildupTime`](/keys/builduptime/) converted to frames and divided by the step count. That count is half the number of frames in the [buildup art](/keys/buildup/), or [`GateStages`](/keys/gatestages/) plus one for a [`Gate=yes`](/keys/gate/) type. That delay is then adjusted by the game-speed setting before it becomes the animation rate. How long a buildup takes therefore tracks the selected game speed as well as the configured value. A type with no buildup art skips the wait and opens at once.

Only when it opens does a structure hand over what came bundled with it. A `FreeUnit` is put down beside it and sent harvesting, and a [`HoverPad=yes`](/keys/hoverpad/) structure receives the first [`PadAircraft`](/keys/padaircraft/) entry unless [`SeparateAircraft=yes`](/keys/separateaircraft/). A free unit that cannot be placed anywhere refunds its own price instead.

## When a factory is lost

Destroying a factory abandons and refunds whatever production was attached to the structure itself. If nothing the house still owns could build what the house's slot was working on, it abandons that too. Capturing one abandons its attached production before the transfer, moves it between the two houses' factory counts, clears its primary flag, and makes both houses re-examine their slots. Switching a factory off or on does the same re-examination.

That re-examination covers the four slots, so it is the player's production it reconciles. It has four outcomes, applied in order:

1. Queued types that nothing the house owns could build even in principle are dropped from the queue.
2. An object in progress that nothing could build any more is abandoned and refunded, and the queue advances.
3. An object that could be built in principle but not right now — typically because every factory for it is switched off — is suspended, keeping its step and its balance.
4. An object that has become buildable again is restarted, unless the player had put it on hold, which is left alone.

A slot left with nothing in progress and nothing queued is discarded. A computer house's own production is not reconciled that way; it ends when the structure holding it is destroyed or captured, or when the structure abandons it for having stopped making progress.

When a computer house cannot make money, it [sells its base for a replacement refinery or harvester](/systems/ai-base-building/#power-and-money-interventions) and abandons every factory it has running as part of that. Handing a house over to the computer abandons them as well.

## Parsed settings without effect

The difficulty blocks have two settings that no production decision reads: [`BuildDelay`](/keys/builddelay/) and [`BuildSlowdown`](/keys/buildslowdown/).
