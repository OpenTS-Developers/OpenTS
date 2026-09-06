---
title: Aim an anti-air defense at a chosen aircraft
category: balance
release: 0.2.0
targets:
- type: key
  id: SAM
  effect: changed
- type: format
  id: spawn-ini
  effect: changed
breaking: false
credit:
- ZivDero
- dkeeton
---

A defense whose weapon reaches only the air takes an attack order against an airborne aircraft
in range. The cursor follows what the weapon can reach, so a landed aircraft, a ground unit
and open ground offer nothing. A launcher so ordered fires at what it is given instead of
choosing for itself, which is the point of the order against a group of aircraft.

`AimableSams` is not read.

dkeeton is credited for the ts-patches patch this follows, whose `AimableSams` gate the
order here does without.
