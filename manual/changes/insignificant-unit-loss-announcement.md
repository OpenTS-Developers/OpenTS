---
title: Say nothing when an insignificant unit dies
category: fix
release: 0.2.0
targets:
- type: key
  id: Insignificant
  effect: changed
- type: command
  id: CenterOnRadarEvent
  effect: changed
credit:
- ZivDero
- Iran
---

A vehicle, soldier or aircraft whose type sets `Insignificant=yes` now dies without EVA announcing a lost unit and without moving the Goto Radar Event location. The hunter seeker is the shipped case: every launch used to report a lost unit.
