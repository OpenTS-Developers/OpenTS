---
title: Let transports carry vehicles
category: feature
release: 0.2.0
targets:
- type: key
  id: IsVehicleTransport
  effect: added
- type: key
  id: SizeLimit
  effect: added
- type: key
  id: Size
  effect: added
  scope: aircrafttype
- type: key
  id: Passengers
  effect: changed
- type: system
  id: transports
  effect: added
- type: format
  id: save-games
  effect: changed
credit: [ZivDero, Rampastring]
---

`IsVehicleTransport=yes` lets a transport carry vehicles as well as infantry, as Red Alert's could.

`Passengers=` is now a budget of space rather than a head count, spent by each passenger's `Size=`. A transport also refuses any single passenger larger than its `SizeLimit=`. Both new keys default to one, so a ruleset that sets neither keeps the head count it had.

A vehicle put out of a transport now lands at the centre of its cell instead of an infantry sub-cell spot, where it drew in the wrong place and could not dock a repair bay. Any passenger, not only infantry, can now survive a destroyed transport if it could have entered the cell the transport stood on.
