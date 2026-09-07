---
title: Read base building from the side a house acts as
category: feature
release: 0.2.0
targets:
- type: key
  id: RegularPowerPlant
  effect: added
- type: key
  id: AdvancedPowerPlant
  effect: added
- type: key
  id: PowerTurbine
  effect: added
- type: key
  id: HunterSeeker
  scope: side
  effect: added
- type: key
  id: AIWallTowers
  effect: added
- type: key
  id: AIBaseDefenseCoefficient
  effect: added
- type: key
  id: AIWallDefense
  effect: added
- type: key
  id: AIWallDefenseCoefficient
  effect: added
- type: key
  id: AIBuildsWalls
  scope: side
  effect: added
- type: key
  id: AIBaseDefensePlaceholders
  effect: added
- type: key
  id: AIBaseDefensesWithWalls
  effect: added
- type: key
  id: GDIHunterSeeker
  effect: changed
- type: key
  id: NodHunterSeeker
  effect: changed
- type: key
  id: GDIPowerPlant
  effect: changed
- type: key
  id: GDIPowerTurbine
  effect: changed
- type: key
  id: NodRegularPower
  effect: changed
- type: key
  id: NodAdvancedPower
  effect: changed
- type: key
  id: WallTower
  effect: changed
- type: key
  id: GDIBaseDefenseCoefficient
  effect: changed
- type: key
  id: NodBaseDefenseCoefficient
  effect: changed
- type: key
  id: GDIWallDefense
  effect: changed
- type: key
  id: GDIWallDefenseCoefficient
  effect: changed
- type: key
  id: NodAIBuildsWalls
  effect: changed
- type: system
  id: ai-base-building
  effect: changed
- type: system
  id: superweapons
  effect: changed
credit: [ZivDero, CCHyper, tomsons26]
---

The computer's base building now comes from the side a house acts as. Each side names its power
plants and turbine, its hunter-seeker drone, the towers it lays along its walls, its defense
budget and placeholder counts, and whether it builds a wall, in the section carrying the side's
own name; the first two sides inherit the rules' `GDI` and `Nod` keys as each file sets them, so
the shipped rules build exactly the bases they always did. A house named neither took GDI's defense budget, received no wall
towers and no threat ring, and answered a power shortage with Nod's plants. A side that names no
plant takes the first `BuildPower` entry its country may own.

CCHyper and tomsons26 are credited for Vinifera's side sections, whose power plant, turbine and
hunter-seeker keys these share.
