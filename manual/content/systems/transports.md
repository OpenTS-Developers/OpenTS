---
title: Transports
summary: "Carries infantry and, where a ruleset allows it, vehicles: how a passenger is admitted, how it boards, and how it gets out again."
category: units-movement
keys:
  - Passengers
  - Size
  - SizeLimit
  - IsVehicleTransport
  - PipScale
  - Pip
  - Loadable
  - DeployTime
  - CrewEscape
---

A transport is any type with a [`Passengers`](/keys/passengers/) budget above zero. Buildings, vehicles and aircraft can all hold passengers; only infantry and vehicles can become one, and nothing can order an aircraft into a hold.

## Being admitted

A passenger that boards under its own power asks the transport the same question every time, so a refusal holds for a player's order, for a computer team loading up, and for the last check as the passenger arrives. A carryall lifting a vehicle is the exception: it takes hold of its load directly and asks nothing, so neither key below restrains it.

The transport answers in this order:

1. No capacity, no sender, or a sender the transport is not allied with — no answer at all. The cursor does not change and no enter order is offered.
2. A unit sender, where the transport is not marked [`IsVehicleTransport`](/keys/isvehicletransport/) — the same silence. Infantry skip this test.
3. The fit: the passenger's [`Size`](/keys/size/) has to be within the transport's [`SizeLimit`](/keys/sizelimit/), and the space already spent plus that size within [`Passengers`](/keys/passengers/). Failing either is a refusal.
4. A vehicle transport standing on a water or shore cell refuses whatever else is true.

At the cursor, two more conditions are settled before the transport is asked at all: a transport that is under way refuses, and so does one belonging to a computer team whose script forbids loading — see [`Loadable`](/keys/loadable/). Both give the no-enter cursor.

## Boarding

Clicking sends the passenger at the transport with an enter order. The two establish radio contact, and the transport may first send the passenger to a staging cell beside it so they do not collide on the way in. The passenger is then allowed to path onto the cell the transport occupies, which no other object may do.

Arriving on that cell, the passenger asks once more whether it still fits. The second question matters once passengers differ in size: without it, two of them could each be told there was room and then both board a hold with space for one. Given room, the passenger leaves the map and attaches to the hold. The door shuts once the space spent reaches the full [`Passengers`](/keys/passengers/) budget.

## Unloading

A transport unloads by deploying. It first turns to face the direction it means to unload toward, then puts passengers out one per pass. Each passenger is offered the eight surrounding cells in turn, starting from the transport's rear, and takes the first it could drive or walk into; a cell lying under a bridge is skipped.

Where in that cell the passenger lands depends on what it is. Infantry take one of the sub-cell spots that let several of them share a cell. A vehicle is put at the cell centre instead, because a vehicle left on a sub-cell spot draws in the wrong place and cannot dock a repair bay.

A passenger with nowhere to go is put back aboard and the transport gives up on the rest.

## Losing the transport

When a transport is destroyed its passengers try to get out where it stood. One that cannot enter that cell is killed with it, which is what keeps a vehicle from being dropped onto ground it could never have driven onto. A transport that explodes, or that was destroyed by something that leaves no survivors, kills its passengers outright.

Carrying passengers also stops a vehicle producing an escaping crew of its own; [`CrewEscape`](/keys/crewescape/) covers that.

## Showing the hold

A transport's pip row shows the hold rather than a scaled quantity, one pip per unit of space, each colored by its passenger's [`Pip`](/keys/pip/). A passenger claims as many pips as it claims space, so a `Size=3` passenger shows three. [`PipScale`](/keys/pipscale/) sets how long the row is.
