---
title: Glossary
description: "The terms the manual uses that read like ordinary English while meaning something narrower, and what each one is commonly taken to mean instead."
---

The manual names things in plain words wherever a plain word is honest, so it carries very few terms of art. The terms below are the exception. Each turns up on many pages, none of them has a short synonym, and every one reads like ordinary English while meaning something much narrower. Every other concept in the manual is either a plain word or defined where it is used.

Each entry says what the term means, what it is commonly mistaken for, and which page owns the full explanation.

## Bridge

A bridge is a cell whose ground carries a deck four height levels above it. That makes the cell two places to stand, the ground and the deck, occupied separately. An object can cross onto the deck even where the ground beneath it would refuse that object. [Height, ramps and bridges](/systems/movement-and-terrain/#height-ramps-and-bridges) works through that test.

The deck does not replace the ground. The ground under a deck keeps its own land type and its own occupants, and the two levels are drawn and occupied separately.

## Campaign

A campaign is an ordered sequence of missions a player advances through, declared as one `[Battles]` entry. Winning one mission tears the scenario down and loads the next inside the same session, and [campaign progression](/systems/campaign-progression/) owns that sequence.

Only the global flags, the spare money, the mission timer, the difficulty and the position in the sequence cross the boundary, and each of them is carried by hand. The usual mistake is to look for the campaign's order in its scenario files: the order lives in `MAPSEL.INI`, and a mission advances only to a stage the current one offers.

## Cell

A cell is the grid unit the map is made of. Ranges, footprints, scan radii and waypoints are all written in cells, and one cell is 256 leptons across, a lepton being the engine's internal distance unit.

A cell is not a single place to stand. It carries several standing places, each occupied separately: infantry are offered three of them, so three infantry share one cell, while a vehicle or a structure takes the whole of it. A cell under a bridge deck carries a second set of places for the deck.

The usual mistake is to picture a cell as a square of screen. It is square on the grid and drawn as a diamond, because the grid is drawn turned through 45 degrees, so a cell's four neighbors run diagonally away from it rather than up, down and across.

[Movement and terrain](/systems/movement-and-terrain/) owns the grid and how its places and heights are read.

## Color scheme

A color scheme is a named recoloring the rules declare in `[Colors]`: one hue, saturation and value, from which the engine builds the palette remaps that color whatever carries the scheme. A country hands its scheme to every house built from it, so the scheme colors everything the house owns, and a projectile or a Tiberium type can name one of its own.

The usual mistake is to treat the scheme as a palette the `[Lighting]` tint keys reshape. Those keys leave house color schemes alone; only an ion storm retints one. [The `Color` key](/keys/color/) owns the declarations and the reads that take a scheme name.

## Country

A country is an entry in the rules `[Houses]` list. Its section supplies a name, a color scheme, a side, and the combat, speed and cost biases a house built from it starts with.

Sides are not countries. GDI and Nod are sides, and each `[Sides]` entry lists the countries that fight under it, as the [rules registration lists](/formats/rules-registries/) set out. A scenario builds its playing houses from countries, and two of them can be built from the same one.

## Crate

A crate is a pickup laid on the map as an overlay in a cell. An infantryman, a walker, a hovercraft or a driven vehicle collects it by committing to enter that cell, and collection applies one result drawn from the crate table.

Crates are overlays, not objects standing in the cell. One drawn into a map's overlay layer survives only in a campaign, because outside one the engine discards every crate the map carries. [Crates](/systems/crates/) covers placement, collection and the results.

## Difficulty

Difficulty is the setting a scenario is played at. It becomes a slot per house, and the slot picks one of three rules sections, `[Easy]`, `[Normal]` or `[Difficult]`, that scales that house's damage, armor, speed, rate of fire, prices and build times.

The section names describe the house reading them, not the player's skill. A house not played by a person is handed the inverse of the setting, so on Easy a computer house reads `[Difficult]`. [Difficulty settings and handicaps](/systems/difficulty/) covers the derivation.

## Firestorm

The word carries two senses. A firestorm wall is the wall of sections a house raises and lowers at once with a charge-draining superweapon. Each raised section destroys what stands in its cell and sweeps the cells around it.

The other sense is the Firestorm add-on: the installed expansion content, which counts as installed once the game finds `FIRESTRM.INI`. A setting described as needing Firestorm means the add-on, not the wall. [The firestorm wall](/systems/laser-fences/#the-firestorm-wall) covers how the wall is raised, charged and drained.

## Frame

A frame is either one image in an art file or one tick of the game clock. In an art file it is one image of a shape or a model, and [building animations](/systems/building-animations/) owns the sequences that step through those images. On the game clock it is the counter the game advances once per pass of its logic, so a rate or a delay written in frames is game time.

The usual mistake is to take a frame count on a key page for artwork. A key that asks for frames is asking for game time, and only a frame sequence in an art file counts pictures.

## Harvester

A harvester is a vehicle that takes the harvest mission by itself. It fills from the cells of Tiberium it drives over and hands the load to a refinery for storage or credits.

It is an ordinary vehicle type carrying `Harvester=yes`, not a distinct kind of object. That flag, rather than the type's name or its artwork, is what makes it harvest. [Harvesting](/systems/tiberium/#harvesting) owns the cycle.

## Height

Height is a ground level rather than a pixel. The step between two cells is measured in levels, and one level is 104 leptons, the engine's distance unit. A bridge deck stands four levels above the ground it spans.

`Height=` in a random-map seed is a size index for the generated map, not a ground level at all. [Movement and terrain](/systems/movement-and-terrain/#height-ramps-and-bridges) owns the first sense.

## House

A house is one owner in a running scenario: one holder of objects, credits, power, production, allegiances and a color, played by a person or by the computer. Every ownership field names one, so the term appears wherever an object's owner, an ally, an enemy or a per-house tally is discussed. A house is built from a country, which supplies the name, the color scheme, the side, and the combat, speed and cost biases the house starts with.

Reading "house" as "side" is the common error. A country names a side and several countries can share one, and GDI and Nod are sides. A house is not its country either: the scenario decides how many houses exist, two of them can be made from the same country, and `ActsLike=` points a house at a different country again. The rules files and a scenario both carry a section called `[Houses]`, and the two do not hold the same thing, since the rules list declares the countries and the scenario list creates the houses that play the mission.

[Rules registration lists](/formats/rules-registries/) owns the country a house is built from and the list a scenario builds it out of.

## Image ID

An Image ID is the name a type's artwork is written under. It is both the `art.ini` section that holds the artwork and the base name the engine builds that type's shape and voxel files from. It starts out as the type's own ID, and `Image=` on the type gives it a different one.

An Image ID is not an ObjectType ID. The two are separate names, and a building type carries a second `Image=` of its own, written inside its `art.ini` section rather than on the type. That one selects the basename of the building's main shape and changes neither ID. [SHP images](/formats/shp/) covers both reads and how a type's shape files are named from its Image ID.

## Ion storm

An ion storm is a global condition a scenario starts and stops: it grounds aircraft and hovercraft, calls lightning down onto cells across the map, and swaps the scenario's lighting to its ion values.

An ion storm is not the Ion cannon. The cannon is a superweapon a house fires once its charge is ready, while the storm is a condition a trigger action, a team mission or a random map sets running. [Ion storms](/systems/ion-storms/) covers how one starts, what its lightning strikes and what it disables.

## Lepton

A lepton is the unit the engine measures distance in. A cell is 256 leptons across on the grid and a terrain height level is 104 leptons, so a distance of 5 cells is 1280 leptons. A few settings are written in leptons rather than in cells, `Acceleration=` among them.

Leptons are not cells, and reading one for the other is the mistake: a horizontal distance converts between the two units by a factor of 256, and a height level by 104. Distances on the map are written in cells, which [movement and terrain](/systems/movement-and-terrain/) owns.

## Limbo

An object in limbo exists and is owned but is not on the map. It holds no cell, is not drawn, takes no logic turn, and can be brought back out again as the object it was. Passengers riding a transport, infantry that have entered a building, and an object a factory has finished but not yet delivered are all in limbo. So is every object between being created and being put down, because objects are created in limbo and placed by being taken out of it.

Conditions across the rules turn on it. A house's power balance, its storage capacity, its prerequisites, its superweapon grants, its radar and its team recruitment all count only objects that are out of limbo, and several targeting scans do the same. A house keeps a second set of tallies that does not, so a limboed object still counts against its type's `BuildLimit` while contributing nothing to any of the above.

Limbo is not destruction, although the two share a step. Putting an object in limbo is also the last thing done to it before it is deleted, so one transition covers both a passenger boarding a transport and an object about to be freed. What separates them is whether anything ever brings the object back.

[Object and type system](/internals/class-hierarchy/) owns the state an object holds while it is owned but off the map.

## Locomotor

Locomotor is the travel routine a mobile object moves by, named on its type by `Locomotor=`. One of ten, it settles more about how the object travels than any other setting on the type.

`Locomotor=` is not fixed for the object's life. A running object can be moved by a different one, and a tunneler leaving a war factory and a passenger in a drop pod both swap it. [Movement and terrain](/systems/movement-and-terrain/#what-each-locomotor-drives-its-speed-from) gives what each routine drives its speed from.

## Mission

Mission means one of two things. On a key page it is what an object is doing: Sleep, Guard, Move, Attack, or one of the other states the [mission enum](/reference/enums/mission/) lists. In a campaign it is one map.

Reading a mission as a map is the common error. The two share the word and nothing else, so a key that asks for a mission is never asking which scenario to load. Team scripts use the word a third time, for one entry of a script's list.

## Movement zone

The phrase carries two meanings, and both are in use.

The first is a movement class — one of ten descriptions of which terrain and which obstacles count as crossable, named on a type by `MovementZone=`. The second is a region of connected ground. The map is divided into zones once for each movement class, and two cells sit in the same movement zone for a class only where something of that class can travel between them. This is the sense meant when two cells are said to sit in different movement zones, and it is what a wall, a laser fence or a collapsed cliff changes when it is said to rebuild the movement zones.

What readers get wrong is that a zone means nothing on its own, because it is comparable only within one class. A stretch of land and the water beside it stay two separate movement zones for a tracked vehicle, and become one for a hovercraft. The claim that two cells share a movement zone therefore means nothing until the class is named as well. [The zone map](/systems/movement-and-terrain/#the-zone-map) owns both meanings.

## Object type

An object type is one registered entry in a rules list. It holds what every object of that kind shares: its ID, its rules section, and the values each of its objects starts from. A runtime instance is one object made from a type in a scenario, and it carries its own state.

An object type is not an object. Editing its section changes every object built from it, and one object's position, orders and condition are written nowhere in the rules files. [Object and type system](/internals/class-hierarchy/) covers the split between shared type data and runtime state.

## Overlay

An overlay is a thing laid in a cell rather than standing in it: a wall, a Tiberium field, a vein, a rock or a crate. A cell holds one, and the overlay replaces the land type the cell would otherwise take.

A map stores them in a layer of its own, `[OverlayPack]`, one entry per cell of the grid. An overlay is not a terrain object, which stands in a cell and occupies a standing place. The wall overlay and the building that becomes it belong to [Walls and gates](/systems/walls-and-gates/).

## Path cost

Path cost is the figure a route search adds up while choosing between routes. Every step into a cell is priced by why that cell can be entered, and the search works toward a cheap total rather than a guaranteed cheapest one. [Route search](/systems/route-search/) owns the prices and the search that adds them up.

Path cost is not distance, since two routes of the same length can cost very different amounts and the longer one is often cheaper. It is not the terrain figure either. The numbers under the land-type sections such as `[Clear]`, `[Road]` and `[Water]` are speed fractions that run in the opposite direction, and the route search reads them only to ask whether a cell can be entered at all.

## Playable area

The playable area is the region `[Map] LocalSize=` declares: the cells the player can see, scroll to and play in.

Readers use "playable area" for the whole map as well, and older prose has meant the whole map by it. This manual calls the whole map the playfield, which `[Map] Size=` declares, and reserves "playable area" for the region inside it. [LocalSize](/keys/localsize/) covers how the region is clipped and what it bounds.

## Projectile

A projectile is what a weapon's shot becomes in flight: an object created on the spot from the bullet type the weapon names in `Projectile=`. The engine's own word for it is a bullet, and it stands in no cell, so nothing runs into it and no scan of a cell finds it. [Projectile flight and impact](/systems/projectile-flight/) covers how it flies, turns and detonates.

`Projectile=` is not an entry in a registration list. It names a rules section directly, and a name no weapon has used before is created there and then, so a misspelling gives a projectile with none of the settings you wrote rather than an error.

## Radar

The radar is the map picture in the side panel: cell colors for the ground, object blips in their owners' colors over them, and radar events over everything else.

It is not a picture of what the tactical view shows. The terrain colors are a true-color picture of the playable area, censored only by a per-pixel shroud test as each pixel is plotted. Fog never removes terrain from it, and only objects drop out under fog. [The radar map](/systems/map-visibility/#the-radar-map) covers what the radar draws and what it refuses to draw.

## Script

Two different things are called a script. A script is an ordered list of up to fifty mission and argument pairs, registered under `[ScriptTypes]` and worked down by a team. Scripting means something else: the trigger events and actions a map fires.

Nothing in a script springs on its own, so a script's missions are not a trigger's actions, and a script holds no event of any kind. [Scripts](/mapping/scripts/) owns the list format.

## Side

Side is a `[Sides]` list entry: a name in the rules mapped to the HouseTypes that belong to it. GDI and Nod are sides, and a country names the side it fights under with `Side=`.

Several countries can name one side, so a side is not a country: a house fights under the side its country names. [Rules registration lists](/formats/rules-registries/) owns the `[Sides]` list.

## Sidebar

Sidebar is the build panel that fills a fixed column against the right edge of the screen. It carries the credit readout, the radar pane, four mode buttons and the power bar, with two strips of cameos beside them: structures on the left, and everything else on the right.

The panel is more than the two cameo strips, and no setting lays it out: its width, the number of strips and the size of a cameo slot are fixed in the engine. [Sidebar and build queue](/systems/sidebar/) covers the cameos and the orders a click starts.

## Skirmish

A skirmish is a match that is neither a campaign mission nor a game against other machines: the player against computer houses, offline. It carries a launch file of its own, and [spawn.ini](/formats/spawn-ini/) owns what that file asks for.

A skirmish ends at the score screen rather than at the campaign's ending. It is not a network game either, so a setting that applies in a game against other machines can be recorded here and never applied.

## Slot

Slot is the engine's word for six unrelated things, and it means nothing on its own. In this manual it always carries its qualifier.

- A difficulty slot is which of the three rules sections a house reads: `[Easy]`, `[Normal]` or `[Difficult]`.
- A weapon slot is one of the three positions a type's weapons sit in, filled by `Primary=`, `Secondary=` and `Elite=`.
- A cameo slot is one place on a sidebar strip, showing one build option.
- An upgrade slot is one of the plugs a structure's `Upgrades=` list fills.
- A Tiberium slot is one entry's place in the `[Tiberiums]` registration list.
- A landing slot is one entry in the `DropPod` list, which supplies the landing animation for one approach direction.

An unqualified "slot" says nothing, because only the surrounding sentence names which of the six is meant. [Difficulty settings and handicaps](/systems/difficulty/#from-the-setting-to-a-slot) owns the first of them.

## Spawn

Spawn has two senses. A spawn is a match launched from a launch file, with its spawn houses and the seat each player holds. Spawning is also the creation of objects, including one object spawning another during play.

A spawn house is not a HouseType: no HouseType is registered under `Spawn1` through `Spawn8`, and a scenario cannot define one. [Client launch file](/formats/spawn-ini/) owns the launch-file sense.

## Superweapon

A superweapon is one rules-declared ability of a house. `[SuperWeaponTypes]` declares the weapons for the whole game, and every house holds its own copy of each with its own countdown and availability.

Each copy charges on its own timer, and firing it delivers an effect wired into the engine where the shot is aimed. The common error is to expect a new section to add a new effect: `Type=` selects one of seven behaviors the engine already has, and a section cannot add to them. [Superweapons](/systems/superweapons/) owns the timers, the granting and the behaviors.

## Tag

A tag is the half of a trigger that rides on something: a scenario object, a map cell, or one of the three lists a scenario keeps. It names the trigger, or the head of a chain of linked triggers, and it carries the persistence.

The persistence decides whether the tag is destroyed, kept, or counted down after the trigger fires, and [trigger springing](/systems/trigger-springing/#tags-triggers-and-events-in-brief) owns the three kinds. A tag is not the trigger: it holds no events and no actions, and every occurrence offered to it is passed along the chain.

## Team

Team is two things: a TeamType and the team built from it. A TeamType is the section in `AI.INI`, `AIFS.INI` or a map that says who owns a team, what it is made of and which Script it runs. The team is the live object built from that section, which holds the members and works down the Script's lines.

The section is not the team, and [AI team execution](/systems/ai-team-execution/) owns the difference.

## Theater

A theater is the tile set a map is drawn with: the artwork, the height-ramp and cliff shapes, and the palette. Its control file, the theater's `Root=` plus `.INI`, assigns tile sets to engine roles, and [Theater control files](/formats/theater-control/) owns the format.

A map names its theater before anything else about it is read, and the archives and palette are mounted then. The word also appears as `Theater=yes` on an art entry. There it means the artwork is named per theater rather than per type, so one structure carries a different file in each. Taking that for the map's tile set is the mistake.

## Threat

Threat names two figures. One is the score target selection gives each candidate, built from five coefficients carried by the type doing the choosing. The other is a per-region threat map each house keeps, which the pathfinder weighs a route against.

The five coefficients come from the chooser's own type, not from the object being targeted. Retuning one changes what that type prefers and nothing about how other types treat it. [Target selection and threat rating](/systems/target-selection/#the-threat-score) owns both figures.

## Tiberium

Tiberium is the registered resource that grows on the ground. The `[Tiberiums]` list declares the types, and each one is laid into a map cell as an overlay that carries one growth stage from 0 through 11. A cell is worth its type's `Value=` times one more than its growth stage, and harvesting takes those stages off into storage or credits.

The [Tiberium](/systems/tiberium/) page owns growth, spreading and harvesting. A blossom tree is not the resource: one that seeds Tiberium into a neighboring cell is worth nothing itself, because the cell's worth comes from the overlay.

## Tile

A tile is one isometric piece of theater artwork that a cell draws. The tile type under a cell supplies that cell's land type. Tiles come in numbered sets, and the theater's [control file](/formats/theater-control/) declares which sets it carries.

A tile is not a cell. A cell is the grid square the game plays on, and the tile is the picture drawn at it.

## Trigger

A trigger is a record a map fires: the house that owns it, the events it waits for, and the actions it runs. Nothing examines it on a schedule. It is offered an occurrence by whatever produced it, and it fires only if every event on it is satisfied during that one offer.

The engine splits the record in two, and [trigger springing](/systems/trigger-springing/) owns the split. The tag is the half that rides on an object, a cell or a list, and it decides what happens after the firing. Reading the trigger as the part that rides on the object is the usual mistake.

## Veterancy

Veterancy is the promotion of an object through the veteran and elite ranks, unlocking per-type abilities and an elite weapon.

Promotion is not counted in kills. Experience is credited as the destroyed type's `Cost` set against the killer's own, so a cheap object takes more kills to promote than an expensive one. Only a type carrying `Trainable=yes` earns anything from its own kills, and a capture earns nothing at all. [Veterancy and promotion](/systems/veterancy/) owns the ranks and what each one changes.

## Voxel

A voxel is one piece of a model-drawn object: a `.VXL` holding the model, divided into layers, and the `.HVA` beside it that places each layer. A type fetches its pieces under base names built from its Image ID, and [VXL and HVA models](/formats/vxl-hva/) covers which pieces a type asks for and what each file holds.

One pair is not the object. A turreted vehicle draws from three of them, and everything the engine draws from shapes rather than models is a different kind of artwork altogether.

## Warhead

A warhead is the damage table a weapon's shot carries, named on the weapon by `Warhead=`. It holds what a blast reaches, what each armor class loses from the figure, and how quickly the figure thins with distance.

The weapon holds how much damage a shot carries, the projectile how it travels, and the warhead what that figure is worth against each target, as [Warheads](/systems/warheads/) sets out. Reading a warhead as the weapon is the usual error: one warhead serves many weapons.

## Waypoint

A waypoint is one numbered cell marker in one of a house's twelve paths. Each path is a numbered list of them, and the path is what an order can be given along.

It is not a destination on its own: a click on a waypoint's cell gives the order along the whole path from that waypoint. The object then repeats the same click at each waypoint in turn. [Waypoint paths](/systems/waypoint-paths/) covers plotting, looping and sending objects along them.
