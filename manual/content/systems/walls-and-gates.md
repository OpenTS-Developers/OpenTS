---
title: Walls, gates and wall towers
summary: "A wall BuildingType turns into a cell overlay when it reaches the map, and gates and wall towers stitch into the run that overlay forms."
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

A wall is built as a structure and stored as terrain. The BuildingType exists so that a wall can be priced, queued and placed. The moment it reaches the map, it is replaced by the overlay its [`ToOverlay`](/keys/tooverlay/) names, written into the cell. Everything afterwards belongs to the cell rather than to any object: connection artwork, damage, ownership and blocking. Gates and wall towers are the exception; they stay real structures and connect to the overlay run around them.

## A wall type and the art entry that completes it

Anyone already writing wall sections can skip to [From structure to overlay](#from-structure-to-overlay).

A wall is declared twice. One section declares the BuildingType, which is what a house prices, queues and places. Another declares the OverlayType a wall cell ends up holding. Each is registered in its own list, `[BuildingTypes]` or `[OverlayTypes]`. What ties the two together is [`ToOverlay=`](/keys/tooverlay/), naming the overlay the building turns into. That assignment is read from the type's art entry rather than from its rules section.

The stock walls give both types the same ID, so one rules section and one art section serve the pair, and each key is read by whichever of the two declares it. `GAWALL`, the GDI concrete wall, is written like this, abridged to the keys this page turns on; the surrounding cost, prerequisite and artwork assignments are omitted:

```ini title="rules.ini"
[GAWALL] ; read once as a BuildingType and once as an OverlayType of the same ID
Wall=yes     ; building: turn into the overlay on placement. Overlay: this cell is a wall
Strength=150 ; building: never used, since it is deleted on placement. Overlay: the per-hit damage threshold
High=yes     ; overlay: stop low-flying fire crossing the cell
Sight=1      ; building: how far placement reveals the map
```

```ini title="art.ini"
[GAWALL]
ToOverlay=GAWALL ; the OverlayType above, named here rather than in the rules
DamageLevels=3   ; overlay: the number of damage stages a segment passes through
```

A damage stage is how many hits a wall cell has taken, counted up from zero. The cell stores the figure, the artwork is drawn from it, and reaching [`DamageLevels`](/keys/damagelevels/) removes the segment. Gates and wall towers are BuildingTypes too, but they have neither `Wall=yes` nor a `ToOverlay=`, which is why they stay on the map as structures.

## From structure to overlay

A [`Wall=yes`](/keys/wall/#scope-buildingtype) BuildingType never becomes a runtime instance. When it is placed, it tests the target cell once more, lays that overlay on the cell and stamps the cell with the placing house. It then reveals the map around it out to the type's [`Sight`](/keys/sight/) and deletes itself. It is never listed among the house's buildings, has no strength of its own, and cannot be selected, repaired or captured.

The overlay then runs a placement test of its own, and it is a different one from what the structure passed. It rejects any cell that already has an overlay of any kind, and it requires the land type to be passable to tracked movement rather than reading the land type's [`Buildable`](/keys/buildable/). A cell that refuses the overlay refuses the placement outright, so the structure stays in the production slot and no cell is changed.

Placement legality for the structure itself is the ordinary building test with one addition for walls. A cell already holding a brick, sandbag or Nod wall accepts a wall building whose `ToOverlay` matches that overlay. Two further conditions apply: the cell is owned by the placing house, and the segment has taken at least one damage stage. The damaged segment is then removed before the fresh one is laid, through the same routine as selling. That routine refuses a computer house's own cells, so the repair works only for a player. For a computer house the old overlay stays put, and the new segment's test then rejects the occupied cell. Rebuilding a damaged segment costs the full price of the wall and returns nothing for what it replaced.

The proximity rule that lets a wall run start away from a structure is covered by [base placement and adjacency](/systems/base-adjacency/#placement-decision-order). For a wall, a scanned cell owned by the same house satisfies the check on its own, with no building standing there. [`WallBuildSpeedCoefficient`](/keys/wallbuildspeedcoefficient/) and the sidebar side of placement are covered by [production](/systems/production/#how-long-it-takes).

:::danger[A wall with no `ToOverlay` crashes on placement]
The conversion reads the type's `ToOverlay` without checking it. A BuildingType with `Wall=yes` and no `ToOverlay=` in its art entry crashes the game the first time one of them reaches a cell it is allowed to occupy.
:::

## Filling the gap to the next wall

Placing a wall by hand also lays the segments between it and the nearest wall of the same kind already standing. The fill is triggered by the placed type having a `ToOverlay` that is itself a wall overlay, not by `Wall=yes`. A type that lays a wall overlay without with the flag still triggers the fill, though what it lays in the gap cells are ordinary structures rather than wall segments.

### Reach and direction

The search runs north, east, south and west only; diagonals are never filled. Each direction walks outward one cell at a time for at most [`GuardRange`](/keys/guardrange/) cells. That value is written in cells, and the fill truncates it to a whole number, so `GuardRange=5` reaches five cells and `GuardRange=5.9` reaches the same five. Three stock BuildingType IDs have the value pinned at 5 cells after their sections are read and cannot be retuned this way; [`GuardRange`](/keys/guardrange/) names them.

### What stops the run

Walking a direction ends on the first of three outcomes:

- A matching anchor: the cell holds the same overlay type and is owned by the same house. Both tests must pass, since a segment of a different wall type, or the same type owned by someone else, is not an anchor.
- A cell that cannot be built on: the walk stops there and that direction contributes nothing, so a gap the engine could not legally close is never partially filled.
- The reach runs out before either happened.

An anchor found in the cell immediately next to the placement produces no fill, since there is no gap between them.

### The filled segments

Each intervening cell receives a freshly created building of the placed type. It is then placed exactly as the first one was: it converts to an overlay, stamps the cell with the same house, and reveals sight around itself. Nothing charges the house for them, so a single paid wall can lay up to four runs of free segments in one placement.

The placement cursor previews the same runs before the click. It uses the same reach, the same four directions and the same anchor test, and is drawn only while the whole cursor sits on legal ground.

## Who owns a wall

A wall's owner is a house index stored on the cell. The overlay itself has no owner, so two adjacent segments of the same type can belong to different houses.

Placement stamps the owner directly. Walls that come from a map's overlay data are placed unowned. The map pass that follows loading assigns each of them to the house whose nearest active, placed building has [`WallOwner=yes`](/keys/wallowner/); a map with no such building anywhere leaves its walls unowned. Ownership decides who may sell a segment, whether the automatic gap fill treats it as an anchor, and whether an attacker sees it as a hostile target.

:::caution[`WallOwner` is rewritten outside campaign]
Skirmish, multiplayer and random-map setup overwrite the value on every country that has a house in the game: a [`MultiplayPassive=yes`](/keys/multiplaypassive/) country is forced to `no` and every other country to `yes`. An authored `WallOwner=` therefore only survives in a campaign game.
:::

## Connection frames

Each wall cell stores a damage stage and a connection frame together in one byte, the stage in the high half and the frame in the low half. The connection frame is four bits, one per cardinal direction, rebuilt whenever a wall in the cell or in one of its four neighbors appears or disappears. A rebuild always touches five cells: the four neighbors and the cell itself. All five are registered for redraw whether or not they hold walls.

A neighboring cell counts as a connection when it holds the same overlay type. Five structures widen that. The table gives each one the wall it continues and the directions the count is taken from. Each direction is read from the wall cell being rebuilt towards the neighbor it tests. The third column is the one to read carefully, since four of the five continue a run along a single axis.

| The neighbor holds, alive | Continues | From |
| --- | --- | --- |
| The [`WallTower`](/keys/walltower/) type | Brick or sandbag wall | Any of the four directions |
| The [`GDIGateOne`](/keys/gdigateone/) type | Brick or sandbag wall | East or west only |
| The [`GDIGateTwo`](/keys/gdigatetwo/) type | Brick or sandbag wall | North or south only |
| The [`NodGateOne`](/keys/nodgateone/) type | Nod wall | East or west only |
| The [`NodGateTwo`](/keys/nodgatetwo/) type | Nod wall | North or south only |

Nothing here reads a flag on the type. A gate stitches into a wall run only because `[General]` names it in one of those four keys, and the axis it stitches along is fixed by which key names it. A [`Gate=yes`](/keys/gate/) type that no key names opens and closes normally and leaves the wall run broken at its ends. The wall tower is the only type that connects from every side, and it connects to brick and sandbag walls only, never to Nod wall. The brick and sandbag family and the Nod wall are fixed overlay positions, not a property of an overlay's own section. A wall overlay registered anywhere else joins only its own kind: none of the four gate keys stitches into it.

### Damage stages with no artwork

Immediately after rebuilding a frame, the engine deletes wall overlays that have reached a damage stage the shipped artwork does not cover. The test is on the stored byte as a whole, not on the stage alone, and that is what confines it to isolated segments. The byte is the stage times sixteen plus the connection frame, so comparing it against `16`, `32` or `48` can only match while the frame half is `0`. A damaged segment with even one connection is passed over.

The rule is keyed to fixed positions in `[OverlayTypes]` rather than to anything the overlay's own section says. The table lists the six positions it covers and the stages at which each one collapses. An overlay registered at any other position is never deleted this way, whatever its own `DamageLevels` allows.

| `[OverlayTypes]` position | Stock ID | Isolated segment is deleted at damage stage |
| ---: | --- | --- |
| 0 | `GASAND` | 1 or 2 |
| 1 | `CYCL` | 2 |
| 2 | `GAWALL` | 2 or 3 |
| 3 | `BARB` | 1 |
| 22 | `FENC` | 1 or 2 |
| 26 | `NAWALL` | 2 or 3 |

The deletion clears the cell's overlay, stored byte and owner, and detaches everything that referred to the cell. It runs without any damage being applied, so an isolated segment can vanish the moment a neighbor is removed and its connection frame drops to zero. Reordering `[OverlayTypes]` moves these rules onto whichever overlays land on those positions.

## Taking damage

### Whether a hit lands

[`Strength`](/keys/strength/#scope-overlaytype) on a wall overlay is a per-hit threshold, not a pool of hit points. Damage at or above the figure always advances the wall by one stage. Damage below it advances the wall only when a random integer from zero through the figure comes out below the damage. A hit that fails that roll accumulates nothing, so the next hit starts from the same threshold. A damage value of `-1` is the instruction to advance the stage unconditionally, and every hit lands unconditionally during scenario initialization.

Five sources reduce a wall. The table sets each one against the damage it hands the threshold test above. Only the first two have a damage figure a rules file can move; the crusher's `-1` and the last two `200`s are fixed in the engine.

| Source | Damage applied |
| --- | --- |
| An explosion whose warhead is [`Wall=yes`](/keys/wall/#scope-warheadtype), or [`Wood=yes`](/keys/wood/) against an overlay with wood armor | The explosion's own damage |
| A wave weapon sweeping the cell | The weapon's [`AmbientDamage`](/keys/ambientdamage/) |
| A crusher vehicle driving over a [`Crushable=yes`](/keys/crushable/#scope-aircrafttype) overlay | `-1`, which destroys the segment outright |
| The cascade described below, against each neighbor at stage zero | `200` |
| A wall tower taken off the map, against each undamaged wall in the four cardinal cells | `200` |

### Stepping through the stages

A landed hit advances the stage by one and then checks two things.

When the new stage is one below [`DamageLevels`](/keys/damagelevels/) and `DamageLevels` is above 2, each of the four cardinal neighbors holding the same overlay type at stage zero is hit for 200 damage. A wall whose `Strength` is 200 or less therefore advances a full stage from that hit. A wall with `DamageLevels=1` or `DamageLevels=2` never cascades at all, and the cascade is only ever one cell deep. The neighbors it hits start at stage zero and can only reach stage one. A further cascade would need a stage of `DamageLevels - 1`, which is 2 or more.

The segment is then removed under **any of**:

- the damage was `-1`;
- the new stage has reached `DamageLevels`;
- **All of:** the new stage is one below `DamageLevels`, and the connection frame is zero.

The last of the three is what makes an isolated segment die one stage before a connected one. With `DamageLevels=1`, the default, the first landed hit takes the stage to 1 and removes the segment immediately.

Removal clears the cell's overlay, stored byte and owner, recalculates the cell, and rebuilds the [movement zones](/glossary/#movement-zone) and radar background. It then rebuilds the connection frames of the four cardinal neighbors and detaches everything that referred to the cell. When an explosion removes a wall, it also clears the cell from every object that was targeting it.

## Crushing, clearing and selling

A crusher vehicle driving onto a wall overlay marked [`Crushable=yes`](/keys/crushable/#scope-aircrafttype) destroys the segment, plays the overlay's [`CrushSound`](/keys/crushsound/#scope-aircrafttype) and rocks the vehicle forward. This ignores ownership and damage stage entirely.

The sell cursor offers a sale over a cell meeting **all of**:

- it is neither shrouded nor fogged;
- it holds a wall overlay;
- its stored owner is a house under player control.

The sale itself then needs **all of**, in this order:

- the cell's stored owner is a house a human is playing;
- the cell holds a wall overlay;
- **All of:** at least one BuildingType in the rules names that overlay in its `ToOverlay`, and the first such type declared is not [`Unsellable=yes`](/keys/unsellable/).

The two ownership tests are not the same test. They coincide in a campaign game. Outside one, the cursor asks whether the cell belongs to the local player's own house, while the sale asks only whether it belongs to a house some human is playing.

Only that first BuildingType is read, so where several of them lay the same overlay, the one declared earliest decides whether the overlay is sellable at all. The sale then clears the cell and rebuilds the connection frames around it.

:::caution[Selling a wall returns nothing]
The routine computes the wall's price and discards it, so a sold segment yields no credits at all. The emergency money the computer raises by selling off its base never includes its walls either, because the human-player test above rejects a computer house's own cells before anything is removed.
:::

## Gates

A gate is a real structure that stands in the wall line and opens for anything friendly wanting to cross it. [`Gate=yes`](/keys/gate/) supplies the door behavior; the four `[General]` keys in the table above supply the wall connection, and the two are independent.

### Placing a gate

Placement clears the footprint first. Every cell of the gate's foundation that is owned by the placing house and holds a brick, sandbag or Nod wall has that wall removed. Any laser fence section of the same house in those cells is folded into the gate. The wall removal is the selling routine, so it succeeds only for a house a human is playing.

A gate's cell test is not the ordinary building one. It takes the branch a laser fence post takes, and that branch differs from the ordinary one in a single term. Where an ordinary building refuses a cell holding any object at all, this one accepts a cell whose only object is a [`LaserFence=yes`](/keys/laserfence/) structure belonging to the placing house. Anything else in the cell refuses it, including a vehicle, an infantryman, a terrain object or any other structure. Both branches then require the cell's standing places to be free: none of them occupied, none of them the destination of something on its way, and no vehicle holding the cell in reserve as it crosses.

After placement, and again when the gate is taken off the map, the two cells capping its run rebuild their connection frames. One is the cell one step before the gate's origin, the other the cell three steps after it, along the axis the naming key fixed. Those offsets are fixed in the engine and assume a three-cell gate; a gate with a different foundation updates the wrong cells.

### Opening

Infantry, walkers, hovercraft and driven vehicles all ask whatever stands in the cell ahead of them to open as they come up on it. The answer depends on what stands there:

- An allied gate is put onto its opening mission and answers `no` until the door reports itself fully open, so the asker waits in place.
- An enemy gate is never asked to open. The asker's own cell test reports it as destroyable, or as impassable when the asker has nothing that can bring a wall down, unless it already stands open.
- Anything that is not a gate answers `yes` at once and reports itself permanently open, which is what keeps every other structure out of this path.

Infantry standing in a gate's cell scatters away unless the gate is open.

### Holding and closing

The door travel in each direction takes [`DeployTime`](/keys/deploytime/) game minutes. Once open, the gate arms a timer of [`GateCloseDelay`](/keys/gateclosedelay/) game minutes; when it expires the door starts closing and the gate returns to idle once it is shut. [`GateDown`](/keys/gatedown/) plays as the door opens and [`GateUp`](/keys/gateup/) as it closes.

The close timer is reloaded from scratch on every pass while anything other than the gate itself stands anywhere in its footprint. A gate therefore holds open indefinitely while traffic is crossing, and only begins to close once the footprint is completely clear. A close already in progress is reversed in place if the gate is asked to open again.

The door frame drawn while the gate is moving is its completion fraction scaled by [`GateStages`](/keys/gatestages/). The frame details, the damaged block and the reversed buildup animation belong to that key and to [production](/systems/production/#buildup).

### A structure that is not a gate

A building put onto the open mission without `Gate=yes` does not run any of this. It assigns itself Guard on the next pass and carries on as an ordinary structure.

## Wall towers

The wall tower is whichever single BuildingType `[General] WallTower=` names. It stays a real structure instead of converting to an overlay, and every behavior below follows from that one name being matched rather than from any flag on the type.

For a house a human is playing, placing one on a friendly brick or sandbag wall removes that wall first, without the sell sound and without refund. A computer house's tower is placed on top of the surviving segment. The tower may be placed there even on an undamaged segment, which no ordinary wall building may do. Once placed it forces the four cells around it to rebuild their connection frames, and from then on it reads as a wall connection from every direction.

Removing one reverses both halves. The four cardinal neighbors rebuild their frames, and each of them still holding an undamaged wall is then hit for 200 damage. Pulling a tower out of a finished wall line therefore damages the run it was holding together, and can start the cascade described above.

Where the computer places towers, and the defense pairing that puts one in front of each planned base defense, are covered by [AI base planning](/systems/ai-base-building/#walls-and-gates).

## Walls in combat and movement

A wall overlay gives its cell the land type its [`Land`](/keys/land/) names, and marks the cell blocked for pathfinding, or crushable when the overlay is [`Crushable=yes`](/keys/crushable/#scope-aircrafttype). The stock wall overlays leave that land type at `Clear` and block through `Wall=yes` alone. What a vehicle or an infantryman reads at that cell then depends on what it is holding, and the table gives the pairing. Each row names the thing entering: the first five rows are a wall cell and the last three a gate's, because a wall and a closed gate answer the same question differently.

| What is entering, and what it holds | What it reads at the cell |
| --- | --- |
| A crusher vehicle, at a `Crushable=yes` wall | Passable, and an allied wall is also reported as friendly and destroyable |
| A vehicle whose primary warhead is `Wall=yes`, or `Wood=yes` against wood armor | Destroyable, or friendly and destroyable when the wall's house is allied |
| Infantry whose primary weapon has a `Wall=yes` warhead | The same pair of results, with no wood alternative |
| Infantry of any kind, at a wall whose stored damage stage equals [`DamageLevels`](/keys/damagelevels/) | A hole, walked through with no further test, since what the infantry holds is never tested |
| Anything else at a wall, including anything unarmed | Impassable |
| A vehicle or an infantryman, at a closed allied gate | Reported as a closed gate, which is what sends it to ask the gate to open |
| An armed vehicle or infantryman, at a closed enemy gate | Destroyable |
| An unarmed vehicle or infantryman, at a closed enemy gate | Impassable |

A vehicle or infantryman whose weapon can bring the wall down targets the cell itself, which is how one ordered through a wall ends up shooting it. One that reads the cell as flatly impassable attacks nothing and simply routes around. The computer's automatic search for walls to shoot, and the difficulty setting that switches it off, are covered by [target selection](/systems/target-selection/#what-each-kind-of-object-considers).

An overlay marked [`High=yes`](/keys/high/#scope-overlaytype) also stops fire crossing it: a projectile that is not itself `High=yes` and is flying below height 100 detonates on reaching the cell. A man set on fire refuses to run into a cell whose land type is `Wall` or which holds a wall overlay at all.
