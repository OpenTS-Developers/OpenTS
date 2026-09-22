---
title: Shroud, fog and the radar map
summary: "How the shroud and the fog of war hide terrain from the player at this machine, and how the radar map draws through them."
category: maps-scenarios
keys:
  - AircraftFogReveal
  - AllowShroudedSubteranneanMoves
  - AllyReveal
  - AttackingAircraftSightRange
  - FlashFrameTime
  - FogRate
  - LeptonsPerSightIncrease
  - LocalSize
  - MoveToShroud
  - RadarCombatFlashTime
  - RadarOff
  - RadarOn
  - RevealByHeight
  - RevealTriggerRadius
  - ShroudGrow
  - ShroudRate
  - Sight
  - Size
  - VeteranSight
related:
  - type: system
    id: power
  - type: system
    id: cloaking
  - type: system
    id: ion-storms
  - type: system
    id: veterancy
  - type: system
    id: crates
  - type: enum
    id: RadarEventType
  - type: action
    id: TACTION_REVEAL_ALL
  - type: action
    id: TACTION_REVEAL_SOME
  - type: action
    id: TACTION_REVEAL_ZONE
  - type: action
    id: TACTION_CREEP_SHADOW
  - type: action
    id: TACTION_RESHROUD
  - type: mission
    id: TMISSION_REVEAL
  - type: mission
    id: TMISSION_RESHROUD
  - type: mission
    id: TMISSION_GOTO_SHROUD
  - type: command
    id: ToggleRadar
  - type: command
    id: CenterOnRadarEvent
---

Two covers hide terrain: the shroud and the fog of war. Every scenario starts fully shrouded. The fog starts off and is switched on by the game options or, in a campaign, by the map. Both covers are stored once, on the map's cells, and both belong to the player at this machine.

The `rules.ini` settings below affect shroud, fog and the radar. The values are examples; each key page gives the default.

```ini title="rules.ini"
[General]
AircraftFogReveal=4             ; fog radius, in cells, of an airborne aircraft with Sight=0
AllowShroudedSubteranneanMoves=no
AttackingAircraftSightRange=8   ; cells a human player's aircraft reveals when it fires near shroud
LeptonsPerSightIncrease=25      ; leptons of height per 10 percent of extra sight
RevealByHeight=yes
RevealTriggerRadius=7           ; cells the Reveal around waypoint action uncovers
VeteranSight=.5                 ; a SIGHT veteran's sight range is multiplied by one more than this
FlashFrameTime=4                ; frames between flashes of a damaged object's radar blip
RadarCombatFlashTime=30         ; frames a damaged object's radar blip keeps flashing

[AudioVisual]
AllyReveal=yes
ShroudGrow=yes
ShroudRate=6                    ; game minutes between shroud regrowth passes
FogRate=.1                      ; game minutes between fog regrowth passes
```

## Whose looks count

A look is the scan an object makes of the cells around it. It uncovers each cell it reaches. The object making it is the looker.

Only looks made for the local player uncover anything. A look made for any other house does nothing unless it is redirected to the local player. These conditions are tested in order, and any one of them redirects the look:

1. A limpet drone belonging to the local player is attached to the looker.
2. The local player has spied on the radar of the looker's house.
3. The looker's house is allied to the local player, and [`AllyReveal=yes`](/keys/allyreveal/).

Computer houses have no shroud or fog. What a computer house knows is recorded on each object as discovery, described under [What the other houses know](#what-the-other-houses-know). The [Goto nearby shroud](/mapping/missions/tmission-goto-shroud/) team mission reads the local player's shroud: it sends its members toward cells that are shrouded for the player at this machine.

## Cell state

Each cell tracks the shroud and the fog separately. For each cover, a cell can be:

- **mapped**, when some part of it is uncovered but a partial piece of the cover may still be drawn over it;
- **clear**, when no part of that cover remains over it.

The partial piece drawn over a mapped cell depends on which of its eight neighbors are still unmapped. Some combinations have no matching artwork, so uncovering one cell also uncovers any neighbor that would need such a piece. One reveal can therefore open several cells.

Every map starts fully dark. Reading a scenario's `[Map]` [`Size`](/keys/size/), which declares the playfield, resets every cell to shrouded and fogged. No setting opens a scenario uncovered. A scenario that starts with the map revealed does it with a trigger.

### Which cell a coordinate is tested against

A raised coordinate is tested against the cell it is drawn over, not the cell it stands on. Every two height levels move the tested cell one step up the screen. At an odd height the coordinate lies between two cells, and it counts as uncovered if either of them is uncovered.

The reverse also applies. When a cell is uncovered, objects standing on high ground that is drawn over it are [discovered](#what-the-other-houses-know) too. The game checks the cells below it on screen, one step at a time:

- the cell itself, if its ground is at most one level high;
- the next cell down the screen, if its ground is one to three levels high;
- the cell after that, if its ground is three to five levels high, and so on.

Fogging a cell walks the same cells and records the structures standing on them.

## Revealing terrain

### Sight range

[`Sight=`](/keys/sight/) counts cells, unlike a weapon's [`Range=`](/keys/range/), which is in leptons. An ordinary look lifts both the shroud and the fog from the cells it reaches. A vehicle, infantryman or structure with `Sight=0` makes no look at all. Only an aircraft does anything with a zero sight range, as [Who looks, and when](#who-looks-and-when) describes.

For vehicles, infantry and structures, two bonuses raise the range, in this order:

1. **Height.** Sight grows by 10 percent for each whole [`LeptonsPerSightIncrease`](/keys/leptonspersightincrease/) leptons of height. Height is measured from the map's lowest level, so standing on a hill counts. One height level is 104 leptons. With the engine default of 50, each level adds two steps: an object one level up sees 20 percent further, and one four levels up sees 80 percent further.
2. **Veterancy.** An object with the `SIGHT` [veteran ability](/systems/veterancy/#abilities) then multiplies that range by [`VeteranSight`](/keys/veteransight/) plus one.

Each step rounds the range down to a whole number of cells. Aircraft get neither bonus.

:::caution[Sight range is capped at ten cells]
A look reaches at most ten cells, whatever `Sight=`, the height bonus and the veteran bonus produce. Any range above ten has no further effect.
:::

:::danger[Keep LeptonsPerSightIncrease above zero]
`LeptonsPerSightIncrease=0` crashes the game with a division by zero on the next look by a vehicle, infantryman or structure.
:::

### The scan

A look is centered on the cell the looker is drawn over, not the cell it occupies. If that cell lies outside the playfield, the look reveals nothing. Otherwise it covers a disc: every playfield cell whose straight-line distance from the center is within the sight range. The test uses the playfield, not the smaller playable area, so a look can reveal cells in the map border.

With [`RevealByHeight=yes`](/keys/revealbyheight/), high ground can block a cell in the disc. For each cell, the game checks the ground height of one probe cell. The cell is revealed only if the probe's ground is no more than three height levels above the looker. To find the probe, take the cell's position relative to the cell the looker stands on, move two cells down the screen, then one cell toward the looker.

Only that probe is checked. Ground between the looker and the cell is not examined, and no setting changes the three-level limit.

With `RevealByHeight=no`, a vehicle or infantryman that has moved one cell rescans only the outer three rings of its disc. With `RevealByHeight=yes`, every look scans the whole disc.

### Who looks, and when

A vehicle, infantryman or structure can look only after it has been inside the playable area, the region [`LocalSize`](/keys/localsize/) declares inside the playfield. It qualifies when it is placed there or reaches the center of a cell there, and it keeps qualifying when it moves out. It loses the qualification if it is placed outside the playable area, for example by leaving a transport there. When the playable area changes, the game rechecks every object against the cell it stands on, so an object the new area leaves outside also loses it.

Outside a campaign, the vehicles, infantry and structures of a house with [`MultiplayPassive=yes`](/keys/multiplaypassive/) never look.

Regular looks happen as follows:

- A vehicle or infantryman looks each time it reaches the center of a cell.
- An infantryman of the local player's looks once a second while it is moving in the air, if its `Sight=` is above zero.
- An aircraft of the local player or an ally looks every 15 frames. It skips the playable-area test, the height bonus and the veteran bonus.

A landed aircraft sees one cell, whatever its `Sight=`.

An airborne aircraft whose type sets `Sight=0` lifts only the fog, within [`AircraftFogReveal`](/keys/aircraftfogreveal/) cells, and only while fog of war is on. This fog-only look works only on cells already out of the shroud. It applies the height test only while the aircraft flies below half the `[General]` [`FlightLevel`](/keys/flightlevel/#scope-global-rules), the rules-wide value, not the aircraft type's key of that name.

An object also looks at once when:

- an aircraft transport sets it down;
- it finishes a teleport, surfaces from a tunnel, or lands from a jumpjet flight;
- it arrives by drop pod;
- it is a structure, and a human player captures it;
- it is a human player's vehicle, infantryman or aircraft, and the playable area grows to include it;
- the local player discovers it.

The discovery look applies only to the player's objects in a campaign, and to every object outside one. It still passes the redirect test under [Whose looks count](#whose-looks-count). A stranger's object therefore reveals nothing by being discovered. Outside a campaign, a structure an ally places uncovers the ground around it as soon as it is placed, while `AllyReveal=yes`.

Terrain objects and animations never look. Only vehicles, infantry, aircraft and structures do.

Several other events reveal ground.

An object that fires can reveal a fixed two cells around itself. The reveal is made for the target's owner, not the firer's. It happens when the target belongs to a human player and either of these holds:

- the firer is not the local player's, and the local player has not discovered it;
- the firer stands on shrouded or fogged ground, and it is not an aircraft of the local player's.

The second case depends only on where the firer stands. It applies to the local player's vehicles, infantry and structures as well.

A human player's aircraft that fires reveals [`AttackingAircraftSightRange`](/keys/attackingaircraftsightrange/) cells around itself when its target is shrouded, or when the shroud lies under it or two cells from it.

Both firing reveals pass the test under [Whose looks count](#whose-looks-count). They uncover ground only when made for the local player, for an ally while `AllyReveal=yes`, or for a house the local player has spied on.

When a house allies with the local player and `AllyReveal=yes`, every object of that house looks at once. The usual rules still apply, so the vehicles, infantry and structures of a [passive house](/keys/multiplaypassive/) that allies outside a campaign reveal nothing.

A spy that enters a [`Radar=yes`](/keys/radar/) structure marks the structure's owner as spied on by the spy's house. If the spy is the local player's, every object of the spied house looks at once. From then on, that house's looks reveal ground for the local player until the mark is removed.

The mark is recomputed from the house's remaining radar structures when a spied radar structure is destroyed, or when the house that spied on it captures it.

### Reveals granted outright

- [Reveal around waypoint...](/mapping/actions/taction-reveal-some/) reveals [`RevealTriggerRadius`](/keys/revealtriggerradius/) cells around a waypoint, with the height test.
- [Reveal zone of waypoint...](/mapping/actions/taction-reveal-zone/) reveals two cells around every playable-area cell that shares the waypoint's crusher [movement zone](/glossary/#movement-zone), with the height test.
- [Reveal all map](/mapping/actions/taction-reveal-all/) lifts both covers from every cell of the playfield.
- [Reveal map](/mapping/missions/tmission-reveal/) lifts the shroud only and leaves the fog.
- An observer's seat, and the local player's defeat outside coach mode, lift both covers from every cell and discard the fog stand-ins. [Observers and coach mode](/systems/observers/) owns that view. Under coach mode, defeat changes nothing.
- The reveal and darkness crate results are described under [crates](/systems/crates/#results-that-reach-the-whole-map).

Reveal all map, Reveal map and the reveal crate each mark the map as fully revealed. Once the map is marked, every reveal in the list above reveals nothing, apart from an observer's seat and defeat. After Reveal map or the reveal crate, Reveal all map can therefore no longer lift the fog they left.

## The fog of war

Whether fog is on depends on the game type:

- In a campaign, fog starts off, and only the map's [`FogOfWar=yes`](/keys/fogofwar/) turns it on.
- In a skirmish or network game, the game options decide, and the map's setting is ignored.

When fog is on, the end of scenario loading fogs every cell the player has not yet uncovered. While fog is on, the shroud is drawn with the fog artwork, so both covers look the same.

A cell that goes under the fog records what stands on it. A structure whose whole footprint is fogged leaves a stand-in on each footprint cell, and the player keeps seeing the structure as it was. A vehicle, infantryman or aircraft leaves no stand-in and is deselected, so it disappears from view. Lifting the fog discards the stand-ins, removing a multi-cell stand-in from every cell it covered.

Looks and most reveals lift the shroud and the fog together. The [Reveal map](/mapping/missions/tmission-reveal/) team mission and the reveal crate lift the shroud alone. A fog-only reveal works only on cells already out of the shroud.

Nothing is fogged for an observer or for a defeated player outside coach mode, and the fog is not drawn for them.

## Losing ground again

Neither regrowth pass runs for an observer or for a defeated player outside coach mode; [observers and coach mode](/systems/observers/#the-whole-map) owns that view.

Both passes spare the ground that the watching objects can still see. The watching objects are:

- the objects on the ground that belong to human players and that the local player has discovered;
- while `AllyReveal=yes`, the structures of computer houses allied to the local player.

A watching object spares ground only if its looks count for the local player, as [Whose looks count](#whose-looks-count) describes. In a network game, an opponent's objects spare nothing unless the opponent is allied while `AllyReveal=yes` or has been spied on.

An allied computer house's vehicles and infantry are not watching objects. They uncover ground only when they move, so regrowth can cover the ground around them while they stand still.

### Shroud regrowth

While [`ShroudGrow=yes`](/keys/shroudgrow/) and [`ShroudRate`](/keys/shroudrate/) is not zero, a shroud pass runs every `ShroudRate` game minutes. The first pass runs as soon as both conditions hold; the interval only spaces the passes after it.

Each pass shrouds the edge of every uncovered area: the cells that are mapped but still show a partial shroud piece. Cells deeper inside an uncovered area are clear, so the pass never touches them. The shroud therefore creeps inward one cell per pass and never reappears in the middle of an uncovered area. The watching objects then look again and uncover whatever they can still see.

[Creep shadow back in](/mapping/actions/taction-creep-shadow/) runs one shroud pass on demand, whatever `ShroudGrow` and `ShroudRate` say.

### Fog regrowth

While fog of war is on and [`FogRate`](/keys/fograte/) is not zero, a fog pass runs every `FogRate` game minutes. The first pass runs as soon as both conditions hold.

Each pass works on the edge of the fog the way the shroud pass works on the edge of the shroud. It fogs the edge cells that no watching object can see, so the fog also creeps inward one cell per pass. The watching objects only keep their cells clear; they do not uncover new ground.

The exception is an allied computer structure while `AllyReveal=yes`. Its look during the pass is an ordinary one, so each fog pass lifts both covers around it.

### Re-shrouding everything

[Reshroud map](/mapping/actions/taction-reshroud/) and its [team mission](/mapping/missions/tmission-reshroud/) return every cell to shrouded and fogged. The watching objects then look again. They also clear the fully-revealed mark, so the reveal actions work again.

## The radar map

### What it draws

The radar's background is a picture of every cell in the playable area. Each cell takes the first color that applies:

1. the radar color of a terrain object on the cell;
2. the bridge color, for a cell under a bridge;
3. the radar color of an overlay that is not Tiberium;
4. the color of the Tiberium growing there;
5. otherwise, the tile's low and high colors, adjusted for the theater and brightened by the cell's height.

Objects appear as blips over that picture, in the owning house's color. Disguised infantry use the local player's color instead.

Where several objects share one radar pixel, only one blip is drawn, and the local player's objects take priority. An object in [limbo](/glossary/#limbo) has no blip. A structure's pixels that fall outside the radar are not drawn; a vehicle, infantryman or aircraft there is drawn on the nearest edge pixel instead. Which other objects appear at all is decided by the radar test in [cloaking and detection](/systems/cloaking/#on-the-radar).

Damage that has any effect makes the damaged object's blip flash for [`RadarCombatFlashTime`](/keys/radarcombatflashtime/) frames. Only the local player's objects flash. The blip switches between its normal and inverted color every [`FlashFrameTime`](/keys/flashframetime/) frames.

A white rectangle marks the part of the map the tactical view is showing, and a white frame surrounds the pane. Radar events are drawn over the terrain and the blips; [radar event](/reference/enums/radar-event/) covers the kinds and their timings.

### What it refuses to draw

:::caution[The radar hides terrain only under the shroud]
The background picture ignores the shroud, the fog and ownership. It holds the true color of every playable-area cell at all times. As each pixel is drawn, the game checks whether the ground under it is shrouded and draws black if it is. That check is the only thing that hides terrain on the radar.
:::

Fog does not hide terrain on the radar. Fogged ground keeps its true color there, even while the tactical view hides it. Only objects drop off the radar under fog.

While the pane is opening or closing, only the frame animation is drawn: 40 frames, four system ticks each. [`RadarOn`](/keys/radaron/) plays as the pane starts to open, and [`RadarOff`](/keys/radaroff/) as it starts to close. Clicks on the pane do nothing unless it is fully open and showing the map.

Whether the pane is raised at all is decided elsewhere. [Power output and drain](/systems/power/#radar) covers the availability test, and [ion storms](/systems/ion-storms/#radar) the suppression that overrides it.

## What the other houses know

Discovery is recorded per object, not per cell. Each object has two flags:

- one records that the local player has discovered it;
- the other records that some other house has discovered it. That flag is shared by all other houses and is never cleared.

When the local player discovers an object of another house, the object's tag springs the [Discovered by player](/mapping/events/tevent-discovered/) event. A discovered object also looks, as [Who looks, and when](#who-looks-and-when) describes.

The local player's flag is cleared in three cases:

- A computer house's object goes into [limbo](/glossary/#limbo), for example by boarding a transport. A human player's objects keep the flag.
- Any object placed outside the playable area starts undiscovered.
- An infantryman whose type has `Sight=0` starts undiscovered, wherever it is placed.

Apart from those cases, outside a campaign the local player discovers every object as soon as it is placed on the map. In a campaign, discovery waits for the covers:

- An object placed on a cell is discovered only if the cell is free of both the shroud and the fog.
- A structure is discovered only if its cell is completely clear of shroud.
- Any object is discovered as soon as its position is no longer shrouded, even under fog. An object discovered this way does not spring Discovered by player and does not look, so the event can be missed for it.

Also in a campaign, a human player's objects do not pick an undiscovered object of another house as an automatic target, unless it is an aircraft.

The shroud also limits orders. An order onto a shrouded cell is refused unless the object's type has [`MoveToShroud=yes`](/keys/movetoshroud/) and the cell lies inside the playable area. An allowed order becomes a plain move, except a patrol waypoint order. In a campaign, an order onto a shrouded object also becomes a plain move for a `MoveToShroud=yes` type and is refused for any other.

A subterranean unit ordered to move onto a shrouded object does nothing unless [`AllowShroudedSubteranneanMoves=yes`](/keys/allowshroudedsubteranneanmoves/). An aircraft ignores that order regardless.

:::note[Nothing hides ground from an opponent]
There is no gap generator, GPS reveal or radar jamming. No structure, weapon or setting lets one house hide uncovered ground from another. A [`Camera=yes`](/keys/camera/) weapon does not reveal ground either.

A stun can still take a player's radar away. An [EM pulse](/systems/emp-pulse/) stuns every building inside its radius, whoever owns it, and a stunned [`Radar=yes`](/keys/radar/) structure stops supplying the radar map for as long as the stun lasts. [Power output and drain](/systems/power/#radar) owns that test.
:::

## Parsed settings without effect

[`BlendedFog`](/keys/blendedfog/) and [`CameraRange`](/keys/camerarange/) are read but change nothing; their pages explain why.
