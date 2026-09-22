---
title: Walls, gates and wall towers
summary: "A wall BuildingType becomes an overlay in its cell when placed, while gates and wall towers stay structures and join the wall runs around them."
category: buildings-economy
keys:
  - Wall
  - ToOverlay
  - GuardRange
  - WallOwner
  - Strength
  - DamageLevels
  - Land
  - High
  - Crushable
  - CrushSound
  - Wood
  - Unsellable
  - Gate
  - GateStages
  - GateCloseDelay
  - DeployTime
  - GateUp
  - GateDown
  - GDIGateOne
  - GDIGateTwo
  - NodGateOne
  - NodGateTwo
  - WallTower
related:
  - type: system
    id: production
  - type: system
    id: target-selection
  - type: system
    id: ai-base-building
  - type: system
    id: base-adjacency
---

A wall is bought and placed as a structure, but placement turns it into an overlay in the target cell. From then on the cell holds everything about that segment: its connections to neighboring segments, its damage and its owner. Gates and wall towers are the exception. They stay structures and join the wall runs around them.

## A wall type and the art entry that completes it

A wall needs two type definitions. The BuildingType is what a house pays for, queues and places. The OverlayType is what the wall cell holds afterwards. List the BuildingType under `[BuildingTypes]` and the OverlayType under `[OverlayTypes]`. [`ToOverlay=`](/keys/tooverlay/) links them by naming the overlay the building turns into. The game reads it from the building's art entry in `art.ini`, not from its rules section.

The stock walls give both types the same ID, so one rules section and one art section serve both, and each type reads the keys it uses. This example is `GAWALL`, the GDI concrete wall, abridged to the keys this page covers. Its cost, prerequisites and artwork are omitted.

```ini title="rules.ini"
[GAWALL] ; read as both a BuildingType and an OverlayType
Wall=yes     ; building: convert to the overlay on placement. Overlay: this cell is a wall
Strength=150 ; overlay: the per-hit damage threshold. The building never stays on the map to use it
High=yes     ; overlay: stop low-flying projectiles at this cell
Sight=1      ; building: how far placement reveals the map
GuardRange=5 ; building: how many cells the automatic gap fill searches
```

```ini title="art.ini"
[GAWALL]
ToOverlay=GAWALL ; the OverlayType above
DamageLevels=3   ; overlay: the number of damage stages a segment passes through
```

A wall cell's damage stage counts the hits that have landed on it, starting at zero. The stage selects the segment's artwork, and the segment is removed when its stage reaches [`DamageLevels`](/keys/damagelevels/).

The connection, replacement and gate rules on this page apply only to three fixed positions in `[OverlayTypes]`: the sandbag wall at position 0, the brick wall at 2 and the Nod wall at 26. Stock rules put `GASAND`, `GAWALL` and `NAWALL` there. [Damage stages with no artwork](#damage-stages-with-no-artwork) adds three more fixed positions. An overlay registered at another position does not get these rules, whatever its section says.

Gates and wall towers are BuildingTypes without `Wall=yes` or `ToOverlay=`, so they stay on the map as structures.

## From structure to overlay

A [`Wall=yes`](/keys/wall/#scope-buildingtype) BuildingType never stays on the map as a structure. When it is placed, it checks the target cell again, puts its overlay in the cell and records the placing house as the cell's owner. It then reveals the map within its [`Sight`](/keys/sight/) and deletes itself. It never appears among the house's buildings, has no hit points, and cannot be selected, repaired or captured.

The overlay then applies a second cell test. It requires land that tracked vehicles can cross, and it does not read the land type's [`Buildable`](/keys/buildable/). It also refuses a cell that still holds a wall, or an overlay without [`BuildableOver=yes`](/keys/buildableover/). If this test fails, the placement fails and the wall stays in its production slot.

A new wall can replace a damaged segment. The structure's cell test is the ordinary building test with one exception: a cell holding a brick, sandbag or Nod wall accepts a wall BuildingType when **all of**:

- its `ToOverlay` names the overlay already in the cell;
- the cell is owned by the placing house;
- the segment has taken at least one damage stage.

Before laying the new segment, the game removes the damaged one the same way selling does. That removal needs a house a human is playing and a wall that [can be sold](#crushing-clearing-and-selling). When it fails, the old segment stays and the overlay test refuses the occupied cell. Computer houses therefore never replace segments, and nobody can replace a segment of an unsellable wall. A replacement costs the full price of the wall, and the removed segment refunds nothing.

[Base placement and adjacency](/systems/base-adjacency/#placement-decision-order) covers the proximity rule that lets a wall run start away from a structure. For a wall, any nearby cell owned by the same house satisfies that rule, even with no building on it. [Production](/systems/production/#how-long-it-takes) covers [`WallBuildSpeedCoefficient`](/keys/wallbuildspeedcoefficient/) and the sidebar side of placement.

:::danger[Give every wall type a `ToOverlay`]
Set `ToOverlay=` in the art entry of every `Wall=yes` BuildingType. If it is missing, the game crashes as soon as one of those walls is placed on a legal cell.
:::

## Filling the gap to the next wall

When a player places a wall from the sidebar, the game also fills the gap between it and the nearest matching segment in each direction. The computer's wall placement does not fill gaps.

The fill applies to any BuildingType whose `ToOverlay` names a wall overlay, with or without `Wall=yes`. Without `Wall=yes`, the gap cells receive ordinary structures of that type instead of wall segments.

### Reach and direction

The search runs north, east, south and west, never diagonally. In each direction it checks at most [`GuardRange`](/keys/guardrange/) cells, truncated to a whole number: `GuardRange=5` and `GuardRange=5.9` both check five cells. A value below `1` fills nothing.

### What stops the run

The search in one direction stops at the first of these:

1. **A matching anchor:** a cell holding the same overlay type and owned by the same house. The cells between the new wall and the anchor are filled.
2. **A cell where the wall building could not be placed.** Nothing is filled in that direction.
3. **The end of the reach.** Nothing is filled in that direction.

A segment of another wall type, or of the same type owned by another house, is not an anchor. It stops the search as a cell where the wall cannot be placed.

### The filled segments

Each gap cell receives a new building of the placed type, placed the same way as the first. A `Wall=yes` type converts to an overlay, gives the cell the same owner and reveals the map around itself. The extra segments are free, so one paid wall can fill up to four gaps.

A gap can still be partly filled. The search accepts any cell where the building could stand, but each segment must then pass the overlay's cell test. A gap cell on buildable land that tracked vehicles cannot cross is left empty while the rest of the gap is filled.

Before the click, the placement cursor previews the same fill, with the same reach, directions and anchor test. The preview appears only while every cell under the cursor is a legal placement.

## Who owns a wall

A wall's owner is stored in the cell, not in the overlay, so two adjacent segments of one type can belong to different houses. The owner decides:

- who can sell the segment;
- whether the automatic gap fill treats it as an anchor;
- whether a vehicle or infantryman blocked by it treats it as an allied or a hostile wall.

Placing a wall records the placing house as the owner. Walls in a map's overlay data start unowned. After the map loads, each one goes to the owner of the nearest active, placed building whose country has [`WallOwner=yes`](/keys/wallowner/). If no such building exists anywhere on the map, the walls stay unowned.

Skirmish, multiplayer and random-map setup rewrite `WallOwner` for every country in the game. It turns the flag off for a [`MultiplayPassive=yes`](/keys/multiplaypassive/) country and on for every other one. The rewrite runs only after the map walls have their owners and does not change them, so `WallOwner` applies as written in every game type. Players' starting units and bases are created after the walls are assigned, so on a skirmish or multiplayer map with no pre-placed buildings, the map's walls stay unowned.

## Connection frames

Each wall segment is drawn with a connection frame of four bits, one for each direction. A bit is set when the neighbor in that direction continues the wall. Whenever a wall appears or disappears, the game rebuilds the frames of that cell and its four neighbors. The cell stores the frame together with the damage stage, as 16 times the stage plus the frame.

A neighbor continues the wall when it holds the same overlay type. Any wall overlay other than brick, sandbag and Nod walls connects only this way. Those three also connect to a neighbor holding one of these structures, still standing:

| The neighbor holds | Continues | Direction from the wall cell to the neighbor |
| --- | --- | --- |
| The [`WallTower`](/keys/walltower/) type | Brick or sandbag wall | Any of the four directions |
| The [`GDIGateOne`](/keys/gdigateone/) type | Brick or sandbag wall | East or west only |
| The [`GDIGateTwo`](/keys/gdigatetwo/) type | Brick or sandbag wall | North or south only |
| The [`NodGateOne`](/keys/nodgateone/) type | Nod wall | East or west only |
| The [`NodGateTwo`](/keys/nodgatetwo/) type | Nod wall | North or south only |

For example, a `GDIGateOne` gate directly east of a brick wall cell connects to it, so an east-west run passes through the gate. A brick wall cell directly north of the same gate does not connect to it.

No flag on the type controls this. A structure joins a wall run only when one of these `[General]` keys names it, and the key decides the axis. A [`Gate=yes`](/keys/gate/) type that no key names still opens and closes, but the wall runs on either side end at it.

### Damage stages with no artwork

A frame rebuild deletes a segment of the overlays below when it has no connections and has reached one of the listed damage stages. These are stages the stock artwork does not cover. A segment with at least one connection is never deleted this way.

| `[OverlayTypes]` position | Stock ID | Segment with no connections is deleted at damage stage |
| ---: | --- | --- |
| 0 | `GASAND` | 1 or 2 |
| 1 | `CYCL` | 2 |
| 2 | `GAWALL` | 2 or 3 |
| 3 | `BARB` | 1 |
| 22 | `FENC` | 1 or 2 |
| 26 | `NAWALL` | 2 or 3 |

The rule belongs to these positions, not to anything in the overlay's section. Reordering `[OverlayTypes]` moves it to whichever overlays land in them. An overlay at any other position is never deleted this way, whatever its `DamageLevels` allows.

The segment needs no hit of its own. A segment at one of these stages vanishes the moment its last connected neighbor is removed, and the deletion clears the cell's overlay and owner.

## Taking damage

### Whether a hit lands

[`Strength`](/keys/strength/#scope-overlaytype) on a wall overlay is a per-hit threshold, not a pool of hit points. A hit of at least that much damage always lands and advances the segment one stage. A smaller hit lands with a chance of its damage in `Strength` plus one: 50 damage against `Strength=150` lands 50 times in 151. Missed hits do not add up, so each hit is tested on its own.

Five sources damage walls. The table gives the damage each one passes to this test. Only the first two can be changed in the rules.

| Source | Damage applied |
| --- | --- |
| An explosion in the cell whose warhead is [`Wall=yes`](/keys/wall/#scope-warheadtype), or [`Wood=yes`](/keys/wood/) against an overlay with wood armor | The explosion's damage |
| A sonic wave from an [`IsSonic=yes`](/keys/issonic/) weapon passing over the cell, whatever its warhead | The [`AmbientDamage`](/keys/ambientdamage/) of the firing object's primary weapon |
| A crusher vehicle driving onto a [`Crushable=yes`](/keys/crushable/#scope-aircrafttype) wall | `-1`, which removes the segment outright |
| The cascade described below, against each neighbor at stage zero | `200` |
| A wall tower taken off the map, against each undamaged wall in the four adjacent cells | `200` |

### Stepping through the stages

Each landed hit advances the segment one stage. Two checks follow.

The first is the cascade. When the new stage is one below [`DamageLevels`](/keys/damagelevels/) and `DamageLevels` is above 2, each adjacent segment of the same overlay type that is still at stage zero takes a 200-damage hit. A wall whose `Strength` is 200 or less always loses a stage to it. The cascade never spreads further, because those neighbors reach stage 1 at most, and a cascade needs a stage of 2 or more. Walls with `DamageLevels=1` or `DamageLevels=2` never cascade.

The second check removes the segment under **any of**:

- the damage was `-1`;
- the new stage has reached `DamageLevels`;
- the new stage is one below `DamageLevels` and the segment has no connections.

A segment with no connections therefore dies one stage earlier than a connected one. At the default `DamageLevels=1`, the first landed hit removes any segment.

Removing a segment clears the cell's overlay and owner, updates the [movement zones](/glossary/#movement-zone) and the radar, and rebuilds the connection frames of the four neighbors. That rebuild can delete a neighbor left without connections, as described under [Damage stages with no artwork](#damage-stages-with-no-artwork). When an explosion removes the wall, objects that were attacking the cell return to their previous mission.

## Crushing, clearing and selling

A [crusher vehicle](/keys/crusher/) that drives onto a [`Crushable=yes`](/keys/crushable/#scope-aircrafttype) wall removes the segment, whatever its owner and damage stage. The overlay's [`CrushSound`](/keys/crushsound/#scope-aircrafttype) plays, and the vehicle rocks forward.

In sell mode, the cursor offers to sell a cell that meets **all of**:

- it is neither shrouded nor fogged;
- it holds a wall overlay;
- its owner is a house under the player's control.

The sale itself requires **all of**:

- the cell's owner is a house a human is playing;
- the cell holds a wall overlay;
- at least one BuildingType names that overlay in its `ToOverlay`;
- the first such type declared is not [`Unsellable=yes`](/keys/unsellable/).

The two ownership tests agree in a campaign. In skirmish and multiplayer, the cursor requires the local player's house, while the sale accepts any house a human is playing.

If several BuildingTypes name the same overlay in `ToOverlay`, only the one declared first decides whether that overlay can be sold. A sale clears the cell and rebuilds the connection frames around it.

:::caution[Selling a wall refunds nothing]
A sold wall segment returns no credits. When the computer sells off its base to raise money, it never sells its walls, because a sale requires a house a human is playing.
:::

## Gates

A gate is a structure that stands in a wall line and opens for allied units. [`Gate=yes`](/keys/gate/) gives it the opening door. Naming it in one of the four `[General]` gate keys connects it to the walls around it, as described under [Connection frames](#connection-frames). The two settings are independent.

### Placing a gate

A gate named by `GDIGateOne` or `GDIGateTwo` can be placed over brick and sandbag wall cells that the placing house owns, whatever their damage. A gate named by `NodGateOne` or `NodGateTwo` can be placed over the placing house's Nod wall cells.

When a `Gate=yes` structure is placed, the brick, sandbag and Nod walls that its house owns in its footprint are removed first. The removal works like selling, so it needs a house a human is playing and a sellable wall. Otherwise the gate stands over the surviving segments, as a computer's gate always does. A laser fence of the same house that crosses the footprint is taken down.

A `Gate=yes` type uses the laser fence post's cell test, which differs from the ordinary building test in one respect. A cell holding a [`LaserFence=yes`](/keys/laserfence/) section of the placing house is accepted. Any other object still refuses the cell, including a vehicle, infantry, a terrain object or another structure. As in the ordinary test, no infantry may stand in the cell or be heading for it, and no vehicle may be passing through it or have it reserved.

When a gate named by one of the four gate keys is placed or taken off the map, the cells at the two ends of its run rebuild their connection frames. These are the cell just before the gate's origin cell and the cell three cells after it, along the gate's axis. The offsets assume a gate three cells long. A gate of any other length rebuilds the wrong cells, and the wall ends beside it do not update until another change nearby rebuilds them.

### Opening

Infantry, walkers, hovercraft and driven vehicles ask a closed allied gate to open as they reach it. The gate starts opening, and the unit waits until the door is fully open.

A closed enemy gate is never asked to open. An armed unit treats it as something to destroy, and an unarmed unit treats it as impassable. An enemy gate that already stands open can be passed.

Idle infantry standing in a gate's cell scatter unless the gate is open.

### Holding and closing

The door takes [`DeployTime`](/keys/deploytime/) game minutes to open and the same time to close. [`GateDown`](/keys/gatedown/) plays as it opens and [`GateUp`](/keys/gateup/) as it closes.

A timer of [`GateCloseDelay`](/keys/gateclosedelay/) game minutes starts when the door starts to open. Reaching the fully open position does not restart it. An unblocked gate therefore stays fully open for `GateCloseDelay` minus `DeployTime`, and a gate whose `DeployTime` is at least its `GateCloseDelay` starts closing as soon as it is fully open.

While the door is fully open and any other object stands anywhere in the gate's footprint, the timer restarts on every check. The gate stays open as long as traffic is crossing. When the footprint is clear and the timer has run out, the door closes and the gate goes idle.

If the gate is asked to open while it is closing, the door reverses from where it is.

While the door moves, the frame drawn follows how far the movement has progressed, out of [`GateStages`](/keys/gatestages/) frames. That key and [production](/systems/production/#buildup) cover the frame details, the damaged frames and the reversed buildup animation.

## Wall towers

The wall tower is the one BuildingType named by `[General] WallTower=`. It stays a structure, and everything below follows from that name; the type has no flag for it.

A tower can be placed on a brick or sandbag wall cell that its house owns, even an undamaged one. An ordinary wall BuildingType can only replace a damaged segment. For a house a human is playing, a sellable segment is removed first, with no sound and no refund. Otherwise the tower stands on top of the segment, as a computer's tower always does.

Once placed, the tower rebuilds the connection frames of the four adjacent cells, and from then on it counts as a connection from every direction.

When the tower is taken off the map, the four adjacent cells rebuild their frames. Each of them that still holds an undamaged wall then takes a 200-damage hit. Removing a tower from a wall line therefore damages the run it joined, and can set off the [cascade](#stepping-through-the-stages).

[AI base planning](/systems/ai-base-building/#walls-and-gates) covers where the computer places towers, including the tower it pairs with each planned base defense.

## Walls in combat and movement

A wall overlay gives its cell the land type its [`Land`](/keys/land/) names. It also marks the cell as blocked for pathfinding, or as crushable if the overlay is [`Crushable=yes`](/keys/crushable/#scope-aircrafttype). The stock wall overlays leave `Land` at `Clear`, so `Wall=yes` alone makes them block.

What a vehicle or infantryman reads at a wall or a closed gate depends on what it is and what it carries. In the table, *destroyable* means it can clear the cell by attacking it, and *friendly and destroyable* is the same result for an allied wall. The first five rows cover walls and the last three cover closed gates.

| What is entering, and what it carries | What it reads at the cell |
| --- | --- |
| A crusher vehicle, at a `Crushable=yes` wall | Passable. An allied wall is also friendly and destroyable |
| A vehicle whose primary warhead is `Wall=yes`, or `Wood=yes` against wood armor | Destroyable, or friendly and destroyable when the wall's house is allied |
| Infantry whose primary weapon has a `Wall=yes` warhead | The same two results, with no wood alternative |
| Any infantry, at a wall whose stored damage stage equals [`DamageLevels`](/keys/damagelevels/). Damage removes a wall at that stage, so only a map's overlay data can leave one there | Passable: the wall is a hole, whatever the infantry carries |
| Anything else at a wall, including anything unarmed | Impassable |
| A vehicle or infantryman, at a closed allied gate | A closed gate, which makes it ask the gate to open |
| An armed vehicle or infantryman, at a closed enemy gate | Destroyable |
| An unarmed vehicle or infantryman, at a closed enemy gate | Impassable |

A vehicle or infantryman that reads a wall as destroyable, or friendly and destroyable, first looks for a new route. If that route still runs into the wall, it attacks the cell, even when the wall is allied. That is how a unit ordered through a wall ends up shooting it. One that reads the cell as impassable attacks nothing and routes around it. [Target selection](/systems/target-selection/#what-each-kind-of-object-considers) covers the computer's automatic search for walls to shoot and the difficulty setting that turns it off.

A [`High=yes`](/keys/high/#scope-overlaytype) overlay also stops projectiles. A projectile that is not itself `High=yes` detonates on reaching the cell if it is less than 100 leptons above the ground.

A burning infantryman does not run into a cell whose land type is `Wall` or that holds any wall overlay.
