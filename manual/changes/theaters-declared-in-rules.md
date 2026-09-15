---
title: Declare extra theaters in the rules
category: feature
release: 0.2.0
targets:
- type: format
  id: rules-registries
  effect: changed
- type: format
  id: theater-control
  effect: changed
- type: key
  id: Root
  effect: added
- type: key
  id: IsoRoot
  effect: added
- type: key
  id: Suffix
  scope: theater
  effect: added
- type: key
  id: MMSuffix
  effect: added
- type: key
  id: ImageLetter
  effect: added
- type: key
  id: IsArctic
  effect: added
- type: key
  id: IsIceGrowthEnabled
  effect: added
- type: key
  id: LowRadarBrightness
  effect: added
- type: key
  id: HighRadarBrightness
  effect: added
- type: key
  id: Theater
  scope: scenarios
  effect: changed
- type: key
  id: NewTheater
  effect: changed
credit: [ZivDero]
---

A `[Theaters]` list in the rules declares the theaters a game has. Each entry names a section with that theater's archives, its artwork extension, its image letter, and whether it has ice or arctic terrain. Rules declaring no such list keep `TEMPERATE` and `SNOW`. A list that names a theater is the whole roster, so a mod may drop, reorder or replace them, and has to write out any it means to keep.

`NewTheater` artwork is renamed wherever its second letter is already some theater's image letter. Six fixed two-letter prefixes were matched before. Every piece of artwork the game ships is renamed exactly as it was, and a mod's own prefixes now reach a theater.

A map naming a theater no rules file declares is reported and played in the first declared theater. It used to load whatever archive names lay in front of the theater table in memory.
