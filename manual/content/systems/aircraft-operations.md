---
title: Aircraft operations
summary: "Coordinates aircraft destinations, landing zones and the conflicts that can prevent a landing."
category: units-movement
keys: []
related:
  - type: system
    id: repair
---

Aircraft use their destination as a landing-zone reservation while they approach a cell. The reservation is not an occupant: it prevents another friendly aircraft from choosing the same landing zone before either one arrives.

## Landing-zone conflicts

Inside the playable area, a landing-zone scan rejects a cell when a foot object is already present. The aircraft performing the scan does not count itself as a blocker, except where the scan is looking for somewhere else for that aircraft to go, and a carryall does not treat the vehicle it is collecting as a blocker. An actual occupant blocks the cell regardless of who owns it or how the houses are allied.

A destination held by another active aircraft also blocks the cell in either of these cases:

- both aircraft belong to the same house;
- each aircraft's house considers the other aircraft allied.

A hostile aircraft's destination does not reserve the cell, and neither does a one-way alliance. The aircraft may still be unable to land there for another reason, and an aircraft that actually reaches the cell blocks it like any other occupant.

## Completing a normal move

An aircraft on the normal move path finishes only after its movement controller reports that it has stopped. Entering the destination cell while movement is still in progress does not make the aircraft idle: the approach advances to landing, the landing zone continues to be checked, and the aircraft remains in that phase until movement finishes or the cell must be validated again. Carryall pickup and drop-off use their own move path instead.

## Unloading cargo

A loaded aircraft cannot accept its self-unload action while any building occupies its current cell. The check treats every building alike, including a helipad or repair facility, and is repeated when the unload mission reaches the ground. If a building appears after a human player issues the order, the unload is canceled; a computer-controlled aircraft instead chooses another landing zone. If the building is gone by the time the mission runs, the ordinary unload proceeds.

A passenger aircraft removes one passenger from the front of its cargo chain for each placement attempt. A successful placement clears that passenger's transport state and leaves the remaining passengers for later attempts. If no adjacent cell accepts the passenger, the passenger is put back at the front of the chain with its transport state intact, so the same passenger remains next to unload rather than being lost from the cargo hold.

## Issuing attack orders without ammunition

A player cannot issue an attack order to an aircraft that has no ammunition and is not airborne. This applies to every grounded aircraft, whether or not it is in contact with a helipad or another reload facility. An airborne aircraft with no ammunition still reaches the ordinary player-fire check, as does a grounded aircraft once it holds at least one round; those cases continue to require player control, a primary weapon and freedom from immobilization.
