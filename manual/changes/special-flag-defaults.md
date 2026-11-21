---
title: Give an omitted map flag its default in every scenario
category: fix
release: 0.2.0
targets:
- type: key
  id: HarvesterImmune
  effect: changed
- type: key
  id: InitialVeteran
  effect: changed
- type: key
  id: Inert
  effect: changed
- type: key
  id: DestroyableBridges
  effect: changed
- type: key
  id: TiberiumGrows
  scope: scenarios
  effect: changed
- type: key
  id: TiberiumSpreads
  effect: changed
credit:
- tinix0
---

A map file that leaves out an entry under `[SpecialFlags]` now gets that entry's built-in default in every scenario. It used to keep the value from the scenario played before it in the same session. A mission played after a network game with the harvester truce could start with the truce on, and a skirmish played after a mission with `InitialVeteran=yes` could start every house's starting units as elite.
