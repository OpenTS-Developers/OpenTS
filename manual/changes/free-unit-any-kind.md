---
title: Hand out an infantryman or an aircraft as a free unit
category: feature
release: 0.2.0
breaking: true
migration:
- Check that every `FreeUnit=` entry names a type the rules define. A name that matches no vehicle, infantry or aircraft type used to create an empty vehicle type and now hands out nothing.
targets:
- type: key
  id: FreeUnit
  effect: changed
credit: [ZivDero, dkeeton]
---

`FreeUnit=` now also takes an InfantryType or an AircraftType, looking the name up among vehicles, then infantry, then aircraft. An infantryman is placed beside the structure like a vehicle. An aircraft is put down on the structure and, on a pad, held there; it replaces a hover pad's free `PadAircraft` even when it cannot be placed and is refunded. Only a vehicle that harvests is sent to harvest, where every free unit used to be.

A name that matches no type now gives nothing and is reported in the debug log. It used to create an empty vehicle type and hand that out.

dkeeton is credited for the ts-patches version this follows.
