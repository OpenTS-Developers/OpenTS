---
title: Production and factories
summary: "How houses order, build, pay for and release objects, and which rules limit what they can build."
category: buildings-economy
keys:
  - AltToRally
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

Players and computer houses produce objects in two different ways.

A player's house has four production slots: one each for infantry, vehicles, aircraft and structures. Each slot builds one object at a time, with a queue behind it, however many factories the house owns. Six war factories do not build six vehicles at once. They only change how long the one vehicle in the slot takes, as [More than one factory](#more-than-one-factory) explains.

A computer house has no production slots. Each of its factory structures builds an object of its own, so six war factories build six vehicles at a time. Every idle factory takes the type its house currently wants for that category, so factories that start together build the same type. The house chooses again each time an object of that category leaves a factory.

In both cases an object takes 54 production steps, and part of its price is charged as the steps advance. The finished object then waits out of play until a factory lets it out.

## What counts as a factory

A BuildingType is a factory when its [`Factory=`](/keys/factory/) names the kind of object it produces.

```ini title="rules.ini"
[MYWEAP] ; example war factory BuildingType
Factory=UnitType
WeaponsFactory=yes
Owner=GDI,Nod
```

When a player orders an object, the house looks for a structure that passes all of these tests, in this order:

- it is on the map;
- it is owned by this house;
- its [`Factory=`](/keys/factory/) names the kind of object being ordered;
- it is switched on;
- it is neither being sold nor queued to be sold;
- the product has not reached its [build limit](#build-limits);
- the factory type's [`Owner=`](/keys/owner/) shares at least one country with the product's `Owner=`;
- **Any of:**
  - the factory's type is not listed in [`BuildConst`](/keys/buildconst/);
  - [`MultiMCV=yes`](/keys/multimcv/) is set;
  - the structure acts for a country that is one of the product's owners.

A structure keeps the country it acts for when it changes hands, so the last test lets a captured construction yard keep building its original owner's structures.

The search takes the first [primary factory](#the-primary-factory) it finds that passes. Without one, it takes the last passing structure. For an aircraft, a pad already in radio contact with an aircraft, such as one parked on it, is used only when no free pad passes.

A computer house does not search. Each of its factories builds whatever the house has chosen next for its category.

:::caution[Use the long names in Factory=]
Write `UnitType`, `InfantryType`, `AircraftType` or `BuildingType`. `Factory=` also accepts the short names `Unit`, `Infantry`, `Aircraft` and `Building`, but a player's factory search never matches them, so such a structure never produces for a player. It still counts toward the [multiple-factory adjustment](#more-than-one-factory) for its category. A computer house's factories accept the short names and produce normally.

Names are matched without regard to case. A name the engine does not recognize leaves the type a non-factory.
:::

### The primary factory

A player's finished objects leave from the primary factory of their kind whenever it passes the factory search.

Toggling the flag on a structure clears it from every other structure of the house on the map whose `Factory=` names the same kind, then sets it on the chosen one. Toggling the current primary clears its flag and leaves the house with no primary. EVA announces a newly set primary to a player-controlled house.

A structure loses the flag when it is captured. An [`IsMobileWar=yes`](/keys/ismobilewar/) structure toggles the flag on itself when it opens.

The cursor that offers the toggle appears only over a structure that passes all of these, in this order:

- it is not stunned by an [EM pulse](/systems/emp-pulse/);
- its `Factory=` names a kind of object;
- it belongs to the local player;
- its house owns more than one factory of that kind;
- for an infantry factory, another structure of the house has `Factory=InfantryType`.

The last test checks only the long name. A second barracks written `Factory=Infantry` counts toward the test above it but not toward this one, so the cursor is not offered.

:::caution[Placing a factory can flag it primary]
A newly placed factory is flagged primary when its house owns more than one structure of the BuildingType listed eighth in `[BuildingTypes]`, whatever that type is. The number of factories does not matter, and no other structure loses its flag, so a house can end up with several primaries of one kind. Use the toggle above to choose the primary deliberately.
:::

## What a house may build

Four gates decide whether a house may build an object type. They are checked in this order, and a type must pass all four.

With [`RecheckPrerequisites=yes`](/keys/recheckprerequisites/), [the sidebar sweep](/systems/sidebar/#what-removes-a-cameo) applies all four gates again and cancels production of any type that fails.

### Tech level

[`TechLevel=-1`](/keys/techlevel/#scope-aircrafttype) makes a type unbuildable. Otherwise the type's level must not exceed the house's tech level. A house's level comes from [its section in the map](/keys/techlevel/#scope-house-per-scenario) and defaults to the scenario number. Outside campaigns, every house the session sets up gets the session's chosen level instead, which starts from [`[MultiplayerDefaults] TechLevel`](/keys/techlevel/#scope-global-rules).

### Prerequisites

Every entry in the type's [`Prerequisite=`](/keys/prerequisite/) list must be satisfied. An entry naming a BuildingType requires the house to own at least one live structure of exactly that type. A structure counts from the moment it is placed, so the prerequisite unlocks during its buildup animation. It still counts while switched off.

Seven group names are satisfied by owning any structure on the matching rules list:

| `Prerequisite=` entry | List read |
| --- | --- |
| `POWER` | [`PrerequisitePower`](/keys/prerequisitepower/) |
| `FACTORY` | [`PrerequisiteFactory`](/keys/prerequisitefactory/) |
| `BARRACKS` | [`PrerequisiteBarracks`](/keys/prerequisitebarracks/) |
| `RADAR` | [`PrerequisiteRadar`](/keys/prerequisiteradar/) |
| `TECH` | [`PrerequisiteTech`](/keys/prerequisitetech/) |
| `GDIFACTORY` | [`PrerequisiteGDIFactory`](/keys/prerequisitegdifactory/) |
| `NODFACTORY` | [`PrerequisiteNodFactory`](/keys/prerequisitenodfactory/) |

:::caution[An upgrade prerequisite checks only the newest structure]
When a prerequisite names an upgrade (a type with [`PowersUpBuilding=`](/keys/powersupbuilding/)), one structure decides the test: the house's newest structure that is on the map, switched on and not being sold, whatever its type. The prerequisite is met only if that structure has the upgrade installed. The same upgrade installed on any other structure does not count.
:::

### Ownership

This gate applies only to structures. A BuildingType passes when both of these hold:

- its `Owner=` list is not empty;
- the house owns a construction yard that is on the map, switched on, not being sold, and acting for one of the countries in that list.

[`MultiMCV=yes`](/keys/multimcv/) removes the construction-yard requirement, both here and in the factory search. [`DoubleOwned=yes`](/keys/doubleowned/) opens the type to every country, but only outside campaign games.

Vehicles, infantry and aircraft skip this gate. The factory search enforces their ownership instead, by requiring the factory's `Owner=` to share a country with the product's.

### Build-limit gate

The last gate is the [build limit](#build-limits). It is the only gate the factory search applies again when an order is placed.

### Computer houses

A computer house faces only the tech-level gate. Every type that passes it counts as buildable, and [base planning](/systems/ai-base-building/) decides what the house actually produces.

## Build limits

The sign of [`BuildLimit=`](/keys/buildlimit/) chooses what the limit counts.

| `BuildLimit=` | Counted | Effect |
| --- | --- | --- |
| Above zero | Objects of the type the house owns now, including one still in production | Losing one frees room for another |
| Zero | Nothing | The type can never be built |
| Below zero | Objects of the type the house has ever produced | Losing one frees nothing |

A positive limit counts more than the objects of the type itself:

- A UnitType with a [`DeploysInto=`](/keys/deploysinto/) structure also counts the house's structures of that type, so construction yards use up an MCV's limit.
- An InfantryType with [`VehicleThief=yes`](/keys/vehiclethief/) also counts the house's vehicles that infantry of its type has entered.

A positive limit counts an object from the moment its production starts. A type the house is already building still counts as buildable, so the build is not canceled when its unfinished object brings the count to the limit.

On the sidebar, a type that has reached a positive limit is drawn darkened. A type whose zero or negative limit is used up loses its cameo.

A player's orders are checked against the limit in two places:

1. The factory search rejects every structure while the product is at its limit, so a new order fails.
2. An order that would join a [queue](#the-queue) is refused when the objects owned or produced, plus the object under construction and everything already queued, reach the limit. At a positive limit, the object under construction is already among the objects owned, so it counts twice and the queue refuses an order one object early. With `BuildLimit=2`, nothing finished and one in production, a second order is refused. For a vehicle, this check does not count deployed structures. It has no case for structures, which are never queued.

:::caution[Build limits do not restrain a computer house]
A computer house's factories never check the limit, so the computer keeps producing a type past it. The limit still applies on a few other computer paths. The most visible is team creation. Outside campaigns, a computer team that names a member at its build limit cannot be created. In a campaign it can still form if the house already owns a recruitable member of that type.
:::

## How long it takes

Build time is counted in game frames. It is computed in this order and truncated to a whole number after each step:

1. Start from the object's [`Cost=`](/keys/cost/#scope-aircrafttype), multiplied by [`BuildSpeed`](/keys/buildspeed/) and by 0.9 frames per credit. A structure uses its `Cost=` as written, which includes the price of any [`FreeUnit`](/keys/freeunit/) or pad aircraft that comes with it. (Repair costs use the price without them.)
2. Multiply by the house's build-time multiplier. That multiplier is fixed when the house is given its difficulty, as the country's [`BuildTime=`](/keys/buildtime/#scope-housetype) times the difficulty's [`BuildTime=`](/keys/buildtime/#scope-difficulty-settings) times [`GameSpeedBias`](/keys/gamespeedbias/). Campaign games leave out the country's `BuildTime=`.
3. Divide by the house's [power multiplier](#power).
4. Multiply by the [multiple-factory adjustment](#more-than-one-factory).
5. For a [`Wall=yes`](/keys/wall/#scope-buildingtype) BuildingType, multiply by [`WallBuildSpeedCoefficient`](/keys/wallbuildspeedcoefficient/).

### Production steps

The result sets the pace of the build, not its exact length. Divided by 54 and truncated, it gives the number of frames between production steps, kept between 1 and 255. The object takes 54 of those steps.

With every multiplier at 1, a unit costing 1000 credits gives 900 frames. Dividing by 54 gives 16 frames per step after truncation, so the unit finishes in 864 frames. At 15 frames a second that is 57.6 seconds, not the full minute its price suggests.

The 1-to-255 range on the step sets the shortest build at 54 frames (3.6 seconds) and the longest at 13,770 frames (about 15.3 minutes). With every multiplier at 1, every object costing under 120 credits takes the minimum, and every object costing 15,300 credits or more takes the maximum.

### More than one factory

Extra factories of a category can shorten its build times, depending on [`MultipleFactory`](/keys/multiplefactory/). The house counts its structures whose `Factory=` names the product's category, including ones switched off or still in buildup. The build time is multiplied once by `1 / ((count - 1) × MultipleFactory)`, so one factory leaves it unchanged.

| Factories | Multiplier at `MultipleFactory=1` | At `MultipleFactory=0.5` |
| ---: | --- | --- |
| 1 | 1 | 1 |
| 2 | 1 | 2 |
| 3 | 0.5 | 1 |
| 4 | 0.333 | 0.667 |

At `MultipleFactory=1`, a second factory changes nothing and a third halves the time. Below 1, a pair of factories builds more slowly than a single one. Above 1, the second factory already speeds production. A value of `0` or below skips the adjustment.

### Power

A house short of power builds more slowly. Its build time is divided by a power multiplier taken from a fixed ladder and never below [`MinProductionSpeed`](/keys/minproductionspeed/). [The production ladder](/systems/power/#production) lists the bands.

The multiplier is refreshed whenever the house's power is recalculated: when a structure is placed, lost, damaged, or switched on or off, even if the power balance does not change. A running build picks up the new step interval at that recalculation. A build also takes a fresh interval when it resumes from hold or suspension. Either way it keeps the step it had reached, so low power slows it in place without restarting it.

### Difficulty and campaign games

The country and difficulty multipliers are combined once, when the house is given its difficulty, not for each order. Campaign games leave out the country's multipliers for both build time and price. The `Cost=` and `BuildTime=` in a country section such as `[GDI]` or `[Nod]` therefore affect only skirmish and multiplayer games.

A difficulty's `Cost=` and `BuildTime=` are independent. `Cost=` changes what everything costs without changing build times, and `BuildTime=` changes build times without changing costs.

## Paying for it

The price is charged in installments as the steps advance. Each installment is the unpaid balance divided by the number of steps remaining, so rounding never gains or loses credits, and the balance is exactly zero when the object is finished.

A build stalls while the next installment is more than the house's credits plus its stored Tiberium. That step is undone, nothing is charged, and the step is tried again after the next interval, so the build continues as soon as the money arrives. A house may start or resume a build it cannot yet afford. The build makes no progress until the money is there.

Putting a build on hold charges nothing and keeps the step it had reached.

Canceling a build refunds what has been paid so far: the price minus the unpaid balance. It also deletes the unfinished object and frees its production slot. The refund uses the price at the moment of canceling, so if a price multiplier changed during the build, the refund does not match what was paid.

## The queue

Only a player's production slots have queues. A computer house's factory holds one object with nothing behind it.

An order placed while its slot is busy joins that slot's queue. The queue holds up to [`MaximumQueuedObjects`](/keys/maximumqueuedobjects/) entries, so at most one more than that number of objects of one category can be on order. An order is dropped if the queue is full or the type has reached its build limit, and a player-controlled house then hears [`ScoldSound`](/keys/scoldsound/).

A slot on hold also queues new orders. The exception is the type on hold: ordering it again resumes the held build instead of queuing a second one.

When the object in progress leaves the factory, is canceled, or becomes unbuildable, the first queued type starts as if it had just been ordered.

Structures never queue. The [sidebar](/systems/sidebar/#clicking-a-build-cameo) refuses a second structure order while another is outstanding, whether it is building, on hold, or finished and waiting to be placed. While a structure is on order, every other structure cameo on the sidebar is drawn darkened.

## Leaving the factory

After the last step the finished object waits in its slot. How it leaves depends on whether a player or a computer house built it.

A player's finished vehicle, aircraft or infantryman is announced and immediately tries to leave through a factory. A finished structure is announced and waits; clicking its cameo enters placement mode.

While every factory of its kind is switched off, a finished vehicle, aircraft or infantryman keeps waiting. Clicking its cameo then only repeats the exit attempt, which fails the same way.

If no factory of the house could build the object any more, even a switched-off one, clicking its cameo cancels and refunds the order, and EVA reports that there is no factory.

An exit attempt either succeeds, is temporarily blocked, or fails permanently. Success releases the object and clears the slot. For a player's vehicle, aircraft or infantryman, either kind of failure cancels and refunds the order, even a blockage the factory would soon have cleared. A structure is never canceled this way; it stays in the slot until it is placed.

A structure placed by hand appears on the chosen cell, plays the placement sound and clears the placement cursor. A firestorm wall, or a structure that lays a wall overlay, also fills the gap between itself and a nearby wall of the same house. If the chosen cell refuses the structure, it stays in the slot.

How a factory lets an object out depends on the factory:

- An aircraft appears on its pad. If the pad is already in radio contact with another aircraft, the new one appears at the edge of the playable area and flies in instead. During an ion storm, it appears on a nearby cell.
- A [`WeaponsFactory=yes`](/keys/weaponsfactory/) structure opens its door and clears whatever stands on its exit cell. While it is still unloading the previous object, it passes the next one to another idle structure of the same type, which lets it out through its own exit. If no such structure is free, the attempt is temporarily blocked.
- Every other factory moves the object out of its footprint to an exit cell beside it. If it finds no exit cell, the attempt fails permanently. A [`GDIBarracks=yes`](/keys/gdibarracks/) or [`NODBarracks=yes`](/keys/nodbarracks/) structure starts the object at its [`ExitCoord`](/keys/exitcoord/) offset when the exit cell is its door cell.
- A factory lets out one vehicle or infantryman at a time. While it is still in radio contact with the last object to leave, the next attempt is temporarily blocked. [`Hospital=yes`](/keys/hospital/), [`Armory=yes`](/keys/armory/) and `WeaponsFactory=yes` structures are exempt.
- A refinery or weeder cannot release what it produces. A vehicle briefly appears south-west of it and heads off to harvest, but the order is then canceled and refunded, which removes the vehicle again. Infantry fail the same way.

A jumpjet infantryman whose route to the rally point calls for flight skips the exit cell and flies straight to the rally point. It releases the factory's radio contact at once, so the next object can leave. Other infantry, and jumpjets with a nearby rally point or none, walk to the exit cell as usual.

For a computer house, a vehicle or infantryman leaving through an exit cell (from any factory except a `WeaponsFactory=yes` one) moves to that cell first. The base position its house chooses for it is queued next and becomes the area it guards. If the house has no valid position, the object only clears the factory.

A computer house's factories retry exits on their own and treat the outcomes differently:

- A temporary blockage waits [`PlacementDelay`](/keys/placementdelay/) minutes before the next attempt.
- A permanent failure cancels and refunds the order.

A computer factory never keeps a build on hold. If its build is put on hold or suspended, the factory cancels and refunds it.

An idle computer factory starts the house's next choice for its category once the house has more than 10 credits, counting stored Tiberium.

### Buildup

A newly placed structure plays its construction animation before it opens. It tells the structure that built it when construction begins, which starts a [`ConstructionYard=yes`](/keys/constructionyard/) structure's [`PreProductionAnim`](/keys/preproductionanim/). It tells the builder again when the animation ends, and then opens.

[`BuildupTime`](/keys/builduptime/) sets the animation's length in minutes. It is divided evenly across the animation's steps: half the number of frames in the [buildup art](/keys/buildup/), or [`GateStages`](/keys/gatestages/) plus one for a [`Gate=yes`](/keys/gate/) type. Like the build times above, the minutes are counted at 15 frames a second. A type with no buildup art opens at once.

A structure hands over what comes with it only when it opens. A [`FreeUnit`](/keys/freeunit/) vehicle or infantryman appears beside the structure, and a free aircraft appears on the structure itself. If a free object cannot be placed anywhere, the house gets its price back instead.

A structure present at scenario start hands over no `FreeUnit`. A player's structure hands it over only if the price paid for the structure exceeds its `Cost=` minus the free unit's `Cost=`. A low enough price multiplier can therefore withhold the free unit. A structure the player did not pay for still hands it over.

A [`HoverPad=yes`](/keys/hoverpad/) structure also receives the first [`PadAircraft`](/keys/padaircraft/) entry, unless [`SeparateAircraft=yes`](/keys/separateaircraft/) is set or the structure's [`FreeUnit`](/keys/freeunit/) is an aircraft. A captured structure hands over neither.

## Rally points

A structure whose [`Factory=`](/keys/factory/) is `UnitType`, `InfantryType` or `AircraftType` can hold a rally point. Select it and click the ground to set one. Holding the force-move key instead gives the structure a move order, which packs up a structure that names an [`UndeploysInto`](/keys/undeploysinto/) type. [`AltToRally=yes`](/keys/alttorally/) swaps the two.

The rally point is not always the clicked cell. The engine records the nearest cell to it where infantry could stand, or where an aircraft could fly for an aircraft factory, searched within the structure's movement zone. If there is no such cell, no rally point is set.

The cursor refuses some cells outright: any cell outside the playable area and, except for an aircraft factory, a cell in another movement zone or on ground that is not passable land.

What the departing object does with the rally point depends on the factory:

- An aircraft heads for it as soon as it appears, whether on its pad or at the edge of the playable area. During an ion storm the aircraft appears on a nearby cell and ignores the rally point.
- A [`WeaponsFactory=yes`](/keys/weaponsfactory/) structure passes it to the vehicle, which heads for it once it has cleared the building. A harvester or weeder then starts harvesting from there.
- A barracks, [`Hospital=yes`](/keys/hospital/) or [`Armory=yes`](/keys/armory/) structure passes it to the infantryman, who walks to the exit cell first and heads for the rally point the next time he is idle.
- Every other factory ignores it. A `Factory=UnitType` structure with [`WeaponsFactory=no`](/keys/weaponsfactory/) accepts a rally point, but the vehicles it releases stop on the exit cell.

If a departing object belongs to a house a player controls and its type cannot be ordered into the shroud, the object ignores a rally point that is still shrouded for that player and leaves as if the factory had no rally point. That covers a type with [`MoveToShroud=no`](/keys/movetoshroud/), the default for aircraft types, and a subterranean type while [`AllowShroudedSubteranneanMoves=no`](/keys/allowshroudedsubteranneanmoves/). The check is made as each object leaves, so the rally point works again once the player uncovers that ground.

A structure stores its rally point as its move destination, so a structure that undeploys passes the rally point to the vehicle it becomes.

## When a factory is lost

Destroying a computer house's factory cancels and refunds whatever that factory was building. For a player, the build in the matching slot is canceled and refunded only if no remaining structure of the house could build it.

Capturing a factory cancels whatever it was building for a computer house before the transfer. The factory then moves from one house's factory count to the other's.

A house re-examines its production slot for a category when a factory of that category is placed, leaves the map, is captured, or is switched on or off. A capture makes both houses re-examine the slot. Only a player's production slots are re-examined, and these rules apply in order:

1. Queued types that nothing the house owns could build, even in theory, are dropped from the queue.
2. An object in progress that nothing could build any more is canceled and refunded, and the queue advances.
3. An object that could be built in theory but not now, because every factory that could build it is switched off, is suspended. It keeps its step and the credits already paid.
4. An object that can be built again restarts, unless the player put it on hold.

A slot left with nothing in progress and nothing queued is then discarded.

A computer house's factory production is not re-examined. It ends when the structure is destroyed or captured, or when the factory cancels a build that has stopped making progress.

When a computer house cannot make money, it [sells its base for a replacement refinery or harvester](/systems/ai-base-building/#power-and-money-interventions) and cancels every build it has running. Handing a house over to the computer cancels its builds as well.
