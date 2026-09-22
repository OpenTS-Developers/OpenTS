---
title: Transports
summary: "How transports admit passengers, take them aboard, and let them out, and what happens to passengers when a transport is destroyed."
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

A vehicle or aircraft with a [`Passengers`](/keys/passengers/) budget above zero is a transport. A structure or infantry type that sets `Passengers` admits nobody. A player can order infantry and vehicles aboard a transport, but not aircraft.

```ini title="rules.ini"
[MYAPC]                  ; a UnitType registered in [VehicleTypes]
Passengers=5             ; hold space shared by every passenger aboard
SizeLimit=2              ; the largest Size admitted to that hold
IsVehicleTransport=yes   ; lets a vehicle board; infantry never needs it
PipScale=Passengers      ; draws one pip per unit of space in the hold
```

## Being admitted

When a player points infantry or a vehicle at an allied transport, the result is the first of these that applies:

1. The transport is moving, or it is on a team whose type does not set [`Loadable=yes`](/keys/loadable/). The passenger gets the cannot-enter cursor.
2. The passenger is a vehicle and the transport does not set [`IsVehicleTransport=yes`](/keys/isvehicletransport/). No enter order is offered, and the cursor shows what it would over any other allied object.
3. The passenger does not fit. Its [`Size`](/keys/size/) must be no larger than the transport's [`SizeLimit`](/keys/sizelimit/), and no larger than the space left in the hold. The space left is `Passengers` minus the `Size` of everyone already aboard. A passenger that fails either test gets the cannot-enter cursor.
4. The transport is a vehicle standing on a water or shore cell. The passenger gets the cannot-enter cursor. An aircraft transport skips this test.
5. Otherwise, the passenger can board.

A soldier with [`Mechanic=yes`](/keys/mechanic/) or [`OmniHealer=yes`](/keys/omnihealer/), pointed at a damaged transport, offers to repair it. Hold force-move to board the transport instead.

A vehicle with a repair weapon offers to repair a damaged vehicle transport even while force-move is held, so it cannot be ordered aboard one.

A computer team loading onto its transport repeats the `IsVehicleTransport` and size tests.

Two ways into a hold skip every test above:

- A carryall lifting a vehicle takes it regardless of the transport keys. [`Totable`](/keys/totable/) decides what it can lift.
- A reinforcement group that includes a transport is created with its passengers already aboard one transport. When the group has an aircraft transport, every member that is not an aircraft transport rides in it, vehicle transports included. Otherwise every member that is not a vehicle transport rides in one vehicle transport. Any other transport of the carrying kind stays outside, empty. The number and sizes of the passengers are not checked against `Passengers` or `SizeLimit`.

## Boarding

An ordered passenger moves onto the cell the transport stands on and is tested again when it arrives. A vehicle repeats the `IsVehicleTransport` and size tests. An infantry passenger repeats the ally and size tests, and at a vehicle transport also the water-or-shore test.

Several passengers can be sent to a transport that has room for only one of them. Only those that still fit when they arrive get in.

A passenger that passes is taken off the map into the hold. An infantry passenger that is refused steps aside and stands guard.

## Unloading

A vehicle transport unloads when it deploys, and an aircraft transport lands before it unloads. The [`Passengers`](/keys/passengers/) page gives when a vehicle can deploy and which bridge positions block it.

A vehicle transport first turns its rear toward an open neighboring cell, usually one near its current rear. It does not always pick the nearest open cell. The cell directly ahead is never picked, the cell to the south only when it is almost directly behind, and some other cells only from some facings. If no open cell can be picked, the transport does not unload.

An open cell is one that infantry could walk into, with room for another soldier and no structure, vehicle, other fixed obstacle or enemy in it.

Passengers then leave one at a time. Each passenger tries the eight neighboring cells in turn, starting behind the transport, and takes the first cell it could drive or walk into. Cells under a bridge are skipped. Infantry take one of the spots that let several soldiers share a cell; a vehicle is placed at the center of its cell.

Each passenger that gets out moves into its cell. If the transport is on a team, the passenger joins that team.

If a passenger finds no cell, it goes back aboard and the transport stops unloading. Everyone still aboard stays there.

## Losing the transport

When a vehicle transport is destroyed, each passenger gets out onto the transport's cell if it could enter that cell. Passengers that cannot are killed with it, so a transport destroyed over water kills every passenger that cannot enter water.

Every passenger is killed when the killing damage was forced or the vehicle fell from a height. Forced damage ignores armor and [`Immune=yes`](/keys/immune/). [Destruction and debris](/systems/destruction-and-debris/#a-vehicle) gives the full order of what a destroyed vehicle does.

An aircraft transport's passengers always die with it.

Each passenger killed with a vehicle transport, or with an aircraft transport shot down in the air, counts as a kill for the attacker and can earn it [experience](/systems/veterancy/#earning-experience).

A vehicle whose artwork sets [`DeathFrames`](/keys/deathframes/) becomes a wreck when it is destroyed, and releases no passengers. Passengers still aboard when the wreck is removed are removed with it.

Any vehicle with `Passengers` above zero produces no escaping crew when it is destroyed, whether or not it carries anyone. [`CrewEscape`](/keys/crewescape/) covers that exclusion. A transport taken by a vehicle thief still [releases the thief](/systems/capture/#stealing-a-vehicle).

## Showing the hold

A transport's pip row shows the hold: one pip for each unit of space. An infantry passenger's pips take the color its [`Pip`](/keys/pip/) sets, and every other passenger's pips are green. A passenger fills as many pips as its `Size`, so a `Size=3` passenger shows three.

[`PipScale`](/keys/pipscale/) and [`MaxPips`](/keys/maxpips/) set how many pips the row has. When the hold has more space than the row has pips, the passengers beyond the last pip are not drawn.
