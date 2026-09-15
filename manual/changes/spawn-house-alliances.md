---
title: Read alliances from the spawn house sections outside campaign
category: feature
release: 0.2.0
targets:
- type: key
  id: Allies
  effect: changed
- type: format
  id: scenario-objects
  effect: changed
credit:
- ZivDero
---

A skirmish or multiplayer map may now write `Allies=` in `[Spawn1]` through `[Spawn8]`, naming spawn houses or countries. The house starting at that position begins the match allied to them, one way, in addition to whatever the launch file arranged. Only a campaign read the key before. A section for a position nobody holds is ignored.
