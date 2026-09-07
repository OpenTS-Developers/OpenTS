---
title: Glossary
description: "Cell, house, limbo, movement zone and path cost: what each term means and what it is repeatedly taken to mean instead."
---

The manual names things in plain words wherever a plain word is honest, so it carries very few terms of art. These five are the exception. Each turns up on many pages, none of them has a short synonym, and every one reads like ordinary English while meaning something much narrower. Every other concept in the manual is either a plain word or defined where it is used.

## Cell

The map is a grid of cells, and a cell is the unit that ranges, footprints, scan radii and waypoints are all written in. One cell is 256 leptons across, a lepton being the engine's internal distance unit, so a figure written in cells is multiplied by 256 as it is read and a coordinate divided by 256 names the cell it stands in.

A cell is not a single place to stand. It carries several standing places, each occupied separately, and three of them are offered to infantry — which is how three infantry share one cell while a vehicle or a structure takes the whole of it. A cell spanned by a bridge carries a second, independent set of them for the deck above, so the ground and the bridge over it are occupied separately.

The usual mistake is to picture a cell as a square of screen. It is square on the grid and drawn as a diamond, because the grid is drawn turned through 45 degrees, so a cell's four neighbors run diagonally away from it on screen rather than up, down and across.

## House

A house is one owner in a running scenario: one holder of objects, credits, power, production, allegiances and a color, played by a person or by the computer. Every ownership field names one, so the term appears wherever an object's owner, an ally, an enemy or a per-house tally is discussed.

A house is built from a country, which supplies the name, the color scheme, the side, and the combat, speed and cost biases the house starts with. The rules files and a scenario both carry a section called `[Houses]`, and the two do not hold the same thing: the rules list declares the countries, and the scenario list creates the houses that play the mission.

Reading "house" as "side" is the common error. A country names a side and several countries can share one; GDI and Nod are sides. A house is not its country either — the scenario decides how many houses exist, two of them can be made from the same country, and [`ActsLike=`](/keys/actslike/) points a house at a different country again: the country whose things it builds, plans and is handed, and whose side it fights under.

## Limbo

An object in limbo exists and is owned but is not on the map. It holds no cell, is not drawn, takes no logic turn, and can be brought back out again as the object it was. Passengers riding a transport, infantry that have entered a building, and an object a factory has finished but not yet delivered are all in limbo — and so is every object between being created and being put down, because objects are created in limbo and placed by being taken out of it.

Conditions across the rules turn on it. A house's power balance, its storage capacity, its prerequisites, its superweapon grants, its radar and its team recruitment all count only objects that are out of limbo, and several targeting scans do the same. A house keeps a second set of tallies that does not, so a limboed object still counts against its type's [`BuildLimit`](/keys/buildlimit/) while contributing nothing to any of the above.

Limbo is not destruction, although the two share a step: putting an object in limbo is also the last thing done to it before it is deleted, so one transition covers both a passenger boarding a transport and an object about to be freed. What separates them is whether anything ever brings the object back.

## Movement zone

The phrase carries two meanings, and both are in use.

The first is a movement class — one of ten descriptions of which terrain and which obstacles count as crossable, named on a type by [`MovementZone=`](/keys/movementzone/). The [movement zone](/reference/enums/movement-zone/) page lists them.

The second is a region of connected ground. The map is divided into zones once for each movement class, and two cells sit in the same movement zone for a class only where something of that class can travel between them. This is the sense meant when two cells are said to sit in different movement zones, and it is what a wall, a laser fence or a collapsed cliff changes when it is said to rebuild the movement zones.

What readers get wrong is that a zone means nothing on its own — it is comparable only within one class. A stretch of land and the water beside it stay two separate movement zones for a tracked vehicle and become one for a hovercraft, so "the same movement zone" is only a claim once the class is named as well.

## Path cost

Path cost is the figure a route search adds up while choosing between routes. Every step into a cell is priced by why that cell can be entered at all: an ordinary step is worth 1, and the prices climb steeply from there, so a cell that has something breakable standing in it is worth tens of ordinary steps and one holding a hidden enemy object is worth a thousand. The search works toward a cheap total rather than a guaranteed cheapest one, which is why a route skirting something breakable usually wins over one that goes through it — and why it sometimes does not, as [Route search](/systems/route-search/) sets out. A cell that cannot be entered at all is not priced — it is left out of the search entirely.

Two things about it are routinely misread. It is not distance: two routes of the same length can cost very different amounts, and the longer one is often cheaper. And it is not the terrain figure. The rules sections named after the land types — `[Clear]`, `[Road]`, `[Water]` and the rest — hold numbers that are also called costs, but those are speed fractions from 0 to 1 in which a larger number is faster and 0 means the ground cannot be crossed at all. They run in the opposite direction from a path cost, where a larger number is worse, and the route search consults them only to ask whether a cell can be entered: a road and rough ground price a step identically. What the figures do afterwards is throttle a vehicle moved by the drive locomotor, and nothing else on the map takes any speed from them at all.
