---
title: Aircraft operations
summary: "How aircraft choose and recheck landing zones, finish moves, unload cargo and accept attack orders."
category: units-movement
keys: []
related:
  - type: system
    id: repair
---

An aircraft's destination cell is its landing zone. While the aircraft flies there, the cell counts as reserved even though nothing stands in it yet.

## Landing-zone conflicts

Aircraft test a landing zone at two points: when they choose it, and repeatedly while they approach it. The approach check is the narrower of the two.

### Choosing a landing zone

Before an aircraft carries out a move, patrol or carryall order, it tests the cell it was sent to. If that cell fails, the aircraft searches outward from it and flies to the first cell that passes. If no cell passes, it keeps the original destination. An aircraft whose team is leaving the map skips the test.

A cell passes when **all of**:

- It lies inside the playable area.
- Nothing stands in it: no infantry, vehicle, aircraft, structure or terrain object.
- Tracked vehicles could drive into it.
- No other aircraft has it as its destination, whichever house that aircraft belongs to.
- It passes the approach check below.

A cell holding the aircraft itself, or an object the aircraft is in radio contact with such as the helipad it is docking at, skips the occupant, tracked-vehicle and destination checks. It must still lie inside the playable area and pass the approach check.

### Checking during the approach

A move or patrol order for any aircraft other than a carryall keeps testing the landing zone while the aircraft flies to it. When the test fails, the aircraft chooses a landing zone again as described above. Cells outside the playable area never fail it.

The approach check fails when **any of**:

- Infantry, a vehicle or another aircraft is in the cell, whichever house owns it. While choosing, a carryall ignores the vehicle it has come to collect.
- Another aircraft is heading for the cell, and either both aircraft belong to the same house or each house considers the other allied.

A hostile aircraft heading for the cell does not interrupt a move or patrol approach, and neither does one whose house is allied in one direction only. Either one still keeps the cell from being chosen, because choosing counts every aircraft's destination.

A carryall flying to a cell rechecks it against the choosing conditions, without the approach check. Any aircraft heading for that cell, hostile or not, makes the carryall choose a landing zone again.

An attacking aircraft uses the same check when it picks a cell to fire from. There it also counts itself, so it never picks the cell it is in or the cell it is already heading for.

## Completing a normal move

A move order ends only when the aircraft has stopped moving. Entering the destination cell while still in motion does not end it. Until the aircraft stops, it keeps checking its landing zone and chooses a new one if the check fails. Carryalls use a separate move sequence, so this does not apply to them.

## Unloading cargo

A loaded aircraft cannot unload while its cell holds a building. This applies to every building, including a helipad or repair facility. Over such a cell, the deploy cursor on the aircraft becomes a no-deploy cursor, and the unload order cannot be given.

The aircraft checks again when it lands to unload and before each passenger leaves. If a building is in its cell at that point:

- An aircraft belonging to a human player drops the unload order and switches to guard.
- A computer player's aircraft takes off and looks for another landing zone.

An aircraft carrying passengers lets them out one at a time, each into a free neighboring cell. If no neighboring cell can take the next passenger, that passenger stays aboard and is the next one the aircraft tries to place. Carryalls set their vehicle down by a different route.

## Issuing attack orders without ammunition

A player cannot order a grounded aircraft with an empty [`Ammo`](/keys/ammo/) pool to attack a unit or structure, or to guard an area. This holds whether or not the aircraft is docked at a helipad or other reload building.

An attack aimed at the ground is still accepted. That covers force-fire at a cell, and a click on an enemy wall its weapon can destroy, a destroyable cliff or a veinhole monster.

An airborne aircraft is not refused on ammunition grounds, and neither is a grounded one holding at least one round. Their attack orders follow the usual rules: the aircraft must be under the player's control, have a primary weapon, and not be stunned by an [EMP](/systems/emp-pulse/).
