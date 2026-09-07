---
title: Decode Move to Cell on the width its file declares
category: fix
release: 0.1.0
targets:
- type: mission
  id: TMISSION_MOVECELL
  effect: changed
- type: key
  id: NewINIFormat
  effect: changed
credit: [ZivDero]
---

A Move to Cell script line split its cell on the old width of 128 columns, which no map wider than that can express, so every such line on a modern map landed somewhere else entirely or off the map. The line now reads its cell on the width the scenario declares, and a number that decodes to a cell outside the map leaves the line without a target instead of aiming the team at a placeholder cell.
