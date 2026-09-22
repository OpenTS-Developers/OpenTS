---
title: Movement and terrain
summary: "Which cells an object may enter, which destinations it can reach, and how fast it crosses each cell."
category: units-movement
keys:
  - Acceleration
  - AccelerationFactor
  - Accelerates
  - AllowBurrowing
  - AutoCrush
  - Buildable
  - Climb
  - CruiseHeight
  - Crushable
  - Crusher
  - DeaccelerationFactor
  - DeployToFire
  - FlightLevel
  - HoverBob
  - HoverDampen
  - IsDropship
  - IsTrain
  - Landable
  - Locomotor
  - MaxBlockCount
  - MovementRestrictedTo
  - MovementZone
  - PitchAngle
  - PitchSpeed
  - PlayerAutoCrush
  - ROT
  - RollAngle
  - SlowdownDistance
  - Speed
  - SpeedType
  - TiltsWhenCrushes
  - TooBigToFitUnderBridge
  - TrackedDownhill
  - TrackedUphill
  - TunnelSpeed
  - TurnRate
  - Weight
  - WheeledDownhill
  - WheeledUphill
  - WobbleDeviation
  - WobblesPerSecond
related:
  - type: enum
    id: LandType
  - type: enum
    id: SpeedType
  - type: enum
    id: MZoneType
  - type: internal
    id: locomotion
  - type: system
    id: walls-and-gates
  - type: system
    id: drop-pods
---

Three separate checks stand between a move order and a moving object:

- A **per-step test** decides whether the object may enter the next [cell](/glossary/#cell).
- A map-wide **zone map** decides whether the destination can be reached at all, before any route is planned.
- A **throttle** decides what fraction of its top speed a vehicle crosses each cell at.

An object that will not go where it is sent has failed one of the first two. An object that arrives slower than its [`Speed=`](/keys/speed/) suggests is being held back by the third.

Four inputs feed these checks, and each reaches only some of them. Changing one without the others can leave the result unchanged.

| Input | What it reaches |
| --- | --- |
| The terrain table in `rules.ini` | The per-step test, the throttle, and (through its `Wheel=` column only) the zone map |
| [`SpeedType`](/keys/speedtype/) | Picks the terrain table column that the per-step test and the throttle read |
| [`MovementZone`](/keys/movementzone/) | Picks which set of zones decides whether a destination is reachable, and nothing else on this page |
| [`Locomotor`](/keys/locomotor/) | How the object travels. Only the drive locomotor applies the terrain figures to speed, and only the tunnel locomotor adds a test of its own to the per-step test |

## The cell

A [cell](/glossary/#cell) is one square of the map grid. Ranges, footprints, scan radii and waypoints are all written in cells. The engine measures distance in [leptons](/glossary/#lepton). One cell is 256 leptons across, and one height level is 104 leptons. A setting that takes a distance in cells is converted to leptons as it is read, so `1.5` becomes 384 leptons, and dividing a lepton position by 256 gives the cell that holds it.

A cell has several places to stand, each occupied separately. Three of them are open to infantry, which is how three infantry share one cell. A vehicle or a structure takes the whole cell. A cell under a bridge has a second, separate set of places for the deck, so the ground under the deck and the deck itself are occupied independently.

## The terrain table

The terrain table is twelve `rules.ini` sections, one per [land type](/reference/enums/land-type/): `[Clear]`, `[Road]`, `[Water]`, `[Rock]`, `[Wall]`, `[Tiberium]`, `[Beach]`, `[Rough]`, `[Ice]`, `[Railroad]`, `[Tunnel]` and `[Weeds]`. Each section holds one figure per [speed type](/reference/enums/speed-type/), `Foot=`, `Track=`, `Wheel=`, `Hover=`, `Winged=`, `Float=`, `Amphibious=` and `Creep=`, plus [`Buildable=`](/keys/buildable/). The engine has no built-in values for any of the 96 figures.

A land type is read only from files that contain its section. If no file declares the section, every figure for that land type is zero and `Buildable` is off. Zero means impassable, not slow, so deleting `[Rough]` closes rough ground to every infantryman and vehicle instead of speeding it up.

When a file does contain the section, each key it writes replaces the figure already in force and each key it leaves out keeps it. A later file or a map can therefore change one entry without restating the rest.

Each figure is used in two ways:

- **Exactly zero** refuses the cell. This is the only use the per-step test makes of the table.
- **Any other value** is a fraction of full speed, where `1` is full speed and smaller is slower. Only a vehicle moved by the drive locomotor is slowed by it. Every other object on the ground, including infantry, hovercraft, mechs and tunnelers, uses the zero test alone.

A value above `1` is cut to `1` as it is read, and nothing corrects a negative value. A value with a `%` sign is divided by 100, so `70%` and `0.7` are the same figure, while a bare `70` is cut to `1`.

```ini title="rules.ini"
[Water] ; the figures the game ships for water, less Float=
Foot=0%
Track=0%
Wheel=0%
Hover=100%
Amphibious=80%
Buildable=no

[MYSKIMMER] ; a UnitType registered in [VehicleTypes]
SpeedType=Hover
MovementZone=AmphibiousCrusher
Locomotor={4A582742-9839-11D1-B709-00A024DDAFD1} ; hover
```

The skimmer needs its first two settings to cross water. `SpeedType=Hover` makes it read `Hover=100%`, which is not zero, so the per-step test lets it onto water. `MovementZone=AmphibiousCrusher` makes water count as reachable on the [zone map](#the-zone-map).

The hover locomotor is not needed to cross water. It moves the skimmer at the same speed whatever the `Hover=` figure is, as long as it is not zero. The drive locomotor would scale that speed by the figure.

Aircraft never read the `Winged` column, because the aircraft cell test treats every cell as clear before it reaches the table. The sections the game ships contain no `Winged=` line, so that column is zero for all twelve land types. A vehicle given `SpeedType=Winged` is therefore refused every cell on the map until a rules file fills the column in.

## Why a cell refuses a vehicle

A vehicle's per-step test runs the checks below in this order. Most checks can refuse the cell outright. The rest record an obstruction, which the route search prices instead of avoiding.

1. **Land type restriction.** A vehicle with [`MovementRestrictedTo=`](/keys/movementrestrictedto/) is refused every cell of any other land type, apart from the tunnel and rail bridge allowances that key describes.
2. **Tunnel direction.** A vehicle may not enter a tunnel cell more than a quarter turn off the tunnel's direction. The same limit applies to a tunnel cell it is leaving, so a vehicle cannot cross a tunnel mouth sideways.
3. **Height step.** A step between cells at different heights is refused unless a ramp or a bridge joins them. [Height, ramps and bridges](#height-ramps-and-bridges) gives the rules.
4. **Map edge.** Once a vehicle has entered the map, cells outside the playable area are refused. A train, a vehicle on the retreat mission and a vehicle in a team that is leaving the map are exempt.
5. **Locomotor.** The tunnel locomotor refuses every cell a subterranean vehicle may not [burrow through](/keys/allowburrowing/). The other nine locomotors accept every cell.
6. **Crate.** In a campaign, a computer-controlled vehicle is refused a cell holding a crate.
7. **Wall.** A wall is refused unless the vehicle can get through it. It gets through a [`Crushable=yes`](/keys/crushable/) wall if it can crush, through [`Crusher=yes`](/keys/crusher/) or the veteran [`CRUSHER` ability](/systems/veterancy/#abilities). It gets through any other wall only if its primary weapon's warhead can destroy that wall. [Walls in combat and movement](/systems/walls-and-gates/#walls-in-combat-and-movement) gives the pairings.
8. **Occupants.** Each object in the cell adds a verdict. Buildings, gates, allied objects moving or standing still, enemy objects, crushable objects and cloaked objects are all weighed. Some occupants never block:
   - the transport the vehicle is boarding, and the repair bay it is entering;
   - the refinery bib a harvester is docking at;
   - an invisible structure, a limpet mine, an open laser fence and an inactive firestorm wall.
9. **Terrain figure.** The cell is refused when its land type's figure in the vehicle's `SpeedType` column is exactly zero. This check comes near the end, so an earlier refusal can hide it.
10. **Reservations.** A cell that another object has claimed for its next step counts as blocked by something moving. If enemy infantry claimed it, a vehicle that can crush infantry treats the cell as clear, and one with a weapon that fires at ground targets treats the infantry as destroyable. Any other vehicle is refused.

A terrain object such as a tree counts as destroyable only when the warhead of the weapon the vehicle would use against it has [`Wood=yes`](/keys/wood/) and the terrain type is not [`Immune=yes`](/keys/immune/#scope-aircrafttype). Otherwise the tree refuses the cell.

An [`IsTrain=yes`](/keys/istrain/) vehicle ignores every verdict except an outright refusal, both while its route is planned and while it drives.

When no check refuses the cell, the test returns the most severe obstruction it found. From least to most severe, the verdicts are:

1. Clear.
2. A cloaked enemy.
3. Something moving through.
4. A closed friendly gate.
5. A friendly obstruction that could be destroyed.
6. An enemy obstruction that could be destroyed.
7. A friendly object standing in the way.
8. Strictly prohibited.

Only the last keeps a route out of the cell. The route search enters a cell with any other verdict at a [path cost](/glossary/#path-cost) that depends on the verdict, so a route can lead through a wall the vehicle is able to destroy. [Route search](/systems/route-search/) gives those costs.

A cell under a bridge is tested either at the deck's height or at ground level. At deck height the terrain figure is skipped, which is how a tracked vehicle crosses a river on a bridge although `[Water] Track=` is `0`. At ground level under the same bridge, the cell is tested exactly as it would be in the open.

### Height, ramps and bridges

Every step compares the height of the cell being left with the height of the cell being entered. A **ramp** here is a sloped tile, the artwork that carries ground from one height level to the next.

- **Same height.** Allowed.
- **One level apart.** Allowed only across a ramp, and the ramp must be the lower of the two cells. Climbing, that is the cell being left; descending, it is the cell being entered.
- **Four levels apart.** The bridge case. Allowed only where the lower cell is spanned by a bridge and the span can be traveled.
- **Any other difference.** Refused.

A cliff two levels high stops every vehicle and every infantryman moving across the ground, whatever its terrain figures or movement zone. A tunneler can pass under it by burrowing.

### Infantry and aircraft answer differently

Infantry run a similar test, with these differences:

- [`MovementRestrictedTo=`](/keys/movementrestrictedto/) does not apply to them.
- Their test never asks the locomotor, so an infantryman moved by the tunnel locomotor is not held to the burrowing test.
- A wall at its last damage stage is a gap they can walk through. A wall at any earlier stage refuses them unless their weapon can destroy walls.
- A computer-controlled infantryman is refused a cell holding a crate in every game mode, not only in a campaign.
- Tiberium gets no separate treatment. Its cells are tested through their land type's figure like any other ground.
- The terrain figure is skipped while the infantryman is being unloaded, as when it steps out of the structure that built it or out of a transport aircraft.

An aircraft's test reads no terrain at all. Height, walls, cliffs, water and occupants do not stop it. One rule remains: in a campaign, a player's aircraft that is not a [loaner](/keys/landable/) is refused a shrouded cell.

## The zone map

Before a route is planned, the engine checks that the destination is reachable on the zone map, which is rebuilt when the ground changes. The zone map is built in three stages:

1. Every cell gets one of seven **blockage ratings**.
2. Each of the ten [movement zone](/reference/enums/movement-zone/) classes accepts some ratings and refuses the others.
3. Each connected run of accepted cells becomes one of that class's [movement zones](/glossary/#movement-zone). A destination outside the object's zone is unreachable.

A cell takes the first rating in this table that fits. The rating ignores the object, its speed type and every terrain column except `Wheel`, so one rating serves every class.

| Rating | Set by |
| --- | --- |
| Outside | The cell lies outside the playable area |
| Crushable | The cell has a [`Crushable=yes`](/keys/crushable/) overlay |
| Blocked | The cell has a wall overlay |
| Impassable | The overlay's land type prices `Wheel=` at exactly zero |
| Water | The land type is `Water` or `Beach`, whatever those sections say |
| Impassable | The cell's land type prices `Wheel=` at `0.01` or below, or an active firestorm wall stands there |
| Partly blocked | A terrain object covers some, but not all, of the places infantry could stand in the cell |
| Blocked | A terrain object covers all of those places |
| Open land | Nothing above applied |

Because only `Wheel=` is read, a land type priced at zero for `Wheel` is rated impassable even when its `Track=` or `Hover=` figure is generous. Every class except `Subterannean` and `Fly` then loses all of that ground.

Water and beach cells are rated by land type alone. A hovercraft therefore needs both a `[Water] Hover=` figure above zero, for the per-step test, and a movement zone that accepts water, for the zone map. Neither setting covers for the other.

The two impassable thresholds differ. A cell whose own land type has a `Wheel=` figure of `0.005` lets a wheeled vehicle step in, but it is rated impassable, so most classes cannot plan a route to it.

The table below shows which classes accept each rating. A movement zone name means nothing more than this row of choices.

| Rating | Movement zones that accept it |
| --- | --- |
| Open land | All ten |
| Crushable | `Crusher`, `Destroyer`, `AmphibiousDestroyer`, `AmphibiousCrusher`, `Subterannean`, `InfantryDestroyer`, `Fly` |
| Blocked | `Destroyer`, `AmphibiousDestroyer`, `Subterannean`, `InfantryDestroyer`, `Fly` |
| Water | `Amphibious`, `AmphibiousCrusher`, `AmphibiousDestroyer`, `Fly` |
| Partly blocked | `Infantry`, `InfantryDestroyer`, `Fly` |
| Impassable | `Subterannean`, `Fly` |
| Outside | None |

`Normal`, `Amphibious` and `Infantry` all refuse crushable ground, so a plain amphibious vehicle routes around a sandbag wall just as an ordinary vehicle does. No class accepts ground outside the playable area, `Fly` included, so no route leads there.

Height also separates zones. The zone map compares each cell with its neighbor, not with the cell where the zone started, so:

- A slope that climbs one level per cell never breaks. A long ramp is one zone from bottom to top.
- A drop of four levels or more always separates the cells on either side of it. Its top and bottom share a zone only if some other connection joins them.
- A drop of two or three levels may or may not separate them. The zone map is filled in two directions, and one direction stops at a two-level difference while the other stops at four, so the result depends on which side the fill reached the drop from.

An intact bridge span or a tunnel then links the zones at its two ends. Bridges and tunnels are the only links between zones that the ground itself keeps apart.

## The route search prices no terrain

The route search adds up a [path cost](/glossary/#path-cost) for each route, and terrain is never part of it. The terrain table reaches the search only through the per-step test, where zero refuses a cell and every other figure is treated the same. A road and a patch of rough ground cost the same, so a route that follows roads does so because of the map's layout, not because the search prefers roads. The terrain figures act later, on the throttle.

[Route search](/systems/route-search/) covers what the search does price, and the search settings that no INI key controls.

## Why it is slower than its Speed says

[`Speed=`](/keys/speed/) takes a value from `0` to `100`. A value above `100` is treated as `100`, so raising a fast type's `Speed=` past that changes nothing.

For a vehicle moved by the drive locomotor, the distance it covers each frame comes from the chain of factors below. Steps 1 to 4 set the **throttle**, a fraction from 0 to 1. Steps 1 to 3 are worked out as the vehicle starts each step into a new cell. The terrain affects only the throttle. Steps 5 to 10 are recalculated every frame.

| Step | Factor |
| --- | --- |
| 1 | The terrain figure for the destination cell's land type |
| 2 | The slope multiplier, when the ground under the destination is higher or lower than the ground under the vehicle |
| 3 | Three quarters, once the vehicle's health has fallen to the yellow condition |
| 4 | On an [`Accelerates=yes`](/keys/accelerates/) vehicle only: at most a fifth while the vehicle is crushing something, otherwise the acceleration ramp toward the throttle from steps 1 to 3 |
| 5 | The type's top speed, reduced by any limpet drone clamped to the vehicle |
| 6 | The owner's [ground speed bias](/systems/difficulty/#how-the-figures-are-combined), which combines the country's, the difficulty setting's and the rules-wide bias |
| 7 | The speed multiplier a [crate](/systems/crates/#results-that-sweep-a-radius) gave the vehicle. Only the first speed crate counts; later ones do not raise it |
| 8 | [`VeteranSpeed`](/keys/veteranspeed/) plus one, for a vehicle that has the [`FASTER` ability](/systems/veterancy/#abilities) |
| 9 | The throttle from steps 1 to 4 |
| 10 | Half, while the vehicle is carrying a captured flag |

Several details change the result:

- Step 1 uses the `[Road]` figures whenever the destination's ground is two or more levels from the height the vehicle travels at, as on a bridge deck. A vehicle therefore keeps its speed on a bridge over water.
- The slope multiplier depends only on speed type. `Track` uses [`TrackedUphill`](/keys/trackeduphill/) and [`TrackedDownhill`](/keys/trackeddownhill/). Every other speed type uses [`WheeledUphill`](/keys/wheeleduphill/) and [`WheeledDownhill`](/keys/wheeleddownhill/), whatever its name suggests.
- If the terrain figure times the slope multiplier is exactly zero, the vehicle moves at half speed instead of stopping.
- Steps 5 to 10 round down to a whole number several times along the way, so a slow type loses proportionally more to each factor than a fast one.
- [`TiltsWhenCrushes`](/keys/tiltswhencrushes/) changes only whether the hull tilts while the vehicle crushes something, not its speed.

The damage penalty in step 3 belongs to the drive locomotor alone. Every other object moves at the same speed damaged or undamaged.

## What each locomotor drives its speed from

[`Locomotor=`](/keys/locomotor/) names one of ten travel routines. Only the drive locomotor uses the terrain figures for speed.

| Locomotor | Speed comes from | Terrain figure |
| --- | --- | --- |
| Drive | The full chain above | Sets the throttle |
| Walk, Mech | Steps 5 to 10 above, with the throttle fixed at full | Ignored |
| Hover | The same, times the hover locomotor's own acceleration ramp | Ignored |
| Tunnel | The same, times [`TunnelSpeed`](/keys/tunnelspeed/) while digging down and coming up; a fixed rate underground | Ignored |
| Fly | The type's top speed times the aircraft's own throttle, and nothing else | Ignored |
| Jumpjet | `[JumpjetControls]` alone; the type's `Speed=` is not read | Ignored |
| Levitate | `[LEVITATION]` alone; the type's `Speed=` is not read | Ignored |
| Drop pod | Its height above the ground, with the rules-wide descent rate as the minimum | Ignored |
| Teleport | Instantaneous | Ignored |

An aircraft's speed therefore ignores the ground speed bias, the crate multiplier, the veteran ability, the flag penalty and the damage penalty. The setting meant to give aircraft a bias of their own is stored but never read; [difficulty settings](/systems/difficulty/#parsed-settings-without-effect) covers it.

The other movement settings on this page each reach only the locomotors listed here:

| Settings | Read by |
| --- | --- |
| [`Accelerates`](/keys/accelerates/), [`AccelerationFactor`](/keys/accelerationfactor/), [`DeaccelerationFactor`](/keys/deaccelerationfactor/) and the slope multipliers | Drive |
| [`PitchAngle`](/keys/pitchangle/), [`PitchSpeed`](/keys/pitchspeed/), [`RollAngle`](/keys/rollangle/), [`FlightLevel`](/keys/flightlevel/), [`IsDropship`](/keys/isdropship/) | Fly |
| [`HoverBob`](/keys/hoverbob/), [`HoverDampen`](/keys/hoverdampen/) | Hover and levitate |
| [`AllowBurrowing`](/keys/allowburrowing/), [`TunnelSpeed`](/keys/tunnelspeed/) | Tunnel |
| [`SlowdownDistance`](/keys/slowdowndistance/) | Fly and drive |
| [`Climb`](/keys/climb/), [`CruiseHeight`](/keys/cruiseheight/), [`WobbleDeviation`](/keys/wobbledeviation/), [`WobblesPerSecond`](/keys/wobblespersecond/), [`TurnRate`](/keys/turnrate/) | Jumpjet, from the shared `[JumpjetControls]` section only |

[`Acceleration`](/keys/acceleration/) is read in three places: in `[JumpjetControls]` for the jumpjet locomotor, in `[LEVITATION]` for the levitate locomotor, and on a projectile's BulletType.

[`ROT`](/keys/rot/) affects each locomotor differently:

- A mech stops until it finishes turning, and so does a driving vehicle without a turret. A driving vehicle with a turret keeps moving while it turns.
- A hovercraft turns at twice its `ROT`. It steers its direction of travel separately from the facing it is drawn at, so it slides through turns instead of pivoting.
- Infantry start with a fixed turn rate instead of their type's `ROT`, and take on their `ROT` the first time they are healed.

A running object can be moved by a locomotor other than the one its type names. A tunneler leaving a war factory, a passenger falling in a drop pod and a jumpjet infantryman on the ground are examples. [Locomotion and piggybacking](/internals/locomotion/) covers the swap.

## Settings that reach no decision

Four settings in this group are read but never change movement:

- [`MaxBlockCount`](/keys/maxblockcount/) keeps a count of blocked frames that nothing acts on.
- [`PlayerAutoCrush`](/keys/playerautocrush/) applies to houses a player commands, but the only check that reads it runs for computer-controlled houses.
- The per-type [`AutoCrush`](/keys/autocrush/) has no effect either.
- [`TooBigToFitUnderBridge`](/keys/toobigtofitunderbridge/) changes how a vehicle is drawn against a bridge deck. It does not restrict movement or keep a vehicle off a bridge cell.

[`Weight`](/keys/weight/) and [`DeployToFire`](/keys/deploytofire/) affect other decisions. `Weight` sets how far a blast rocks a voxel object and whether a vehicle breaks the ice beneath it. `DeployToFire` limits where a vehicle may fire from, using each land type's [`Buildable=`](/keys/buildable/) flag.
