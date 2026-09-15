---
title: Read computer base plans from the spawn house sections
category: feature
release: 0.2.0
targets:
- type: key
  id: UseMPAIBaseNodes
  effect: added
- type: system
  id: ai-base-building
  effect: changed
- type: key
  id: NodeCount
  effect: changed
- type: format
  id: scenario-objects
  effect: changed
credit:
- ZivDero
- Rampastring
- CCHyper
---

A skirmish or multiplayer map that sets `[Basic] UseMPAIBaseNodes=yes` hands each house the base node list written in the section of the start position it holds, `[Spawn1]` through `[Spawn8]`. A computer house then follows that plan as a campaign house does. Its nodes go where they are written, no power plant is inserted ahead of them, it does not sell its base to raise money, and it rebuilds a lost defense in place. Maps without the key play as before.

Rampastring and CCHyper are credited for the Vinifera implementation this one follows.
