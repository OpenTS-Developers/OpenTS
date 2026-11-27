---
title: Shared control
summary: A network game option that lets a player give an ally command of their vehicles, infantry, aircraft and structures.
category: multiplayer-networking
keys: []
related:
  - type: command
    id: GrantControl
  - type: format
    id: spawn-ini
  - type: system
    id: network-packet-validation
  - type: system
    id: observers
---

With shared control on, a player can let an ally select and order their vehicles, infantry, aircraft and structures. The owner keeps full command of them, and each grant goes one way: an ally who receives control does not give any back.

## Turning it on

Shared control works only in network games, and only when `SharedControl=yes` is set in the `[Settings]` section of [`spawn.ini`](/formats/spawn-ini/). Every machine in the match needs the same value. A machine with the option off refuses grants and the orders that rely on them while the others carry them out, so the match goes out of sync.

## Granting and withdrawing control

To grant control, select an object that belongs to the ally and press the [`GrantControl`](/commands/grantcontrol/) key, or tick the ally's Give box on the [diplomacy panel](/systems/diplomacy/#shared-control). The command has no default key. Pressing it again with that ally's object selected, or clearing the box, withdraws the grant. Both players see a message when a grant starts or ends.

A grant can be made or withdrawn only when all of these hold:

- Neither player has been defeated or is an observer.
- The other player is human.
- The two players are allied with each other in both directions.

A grant ends when either player breaks the alliance, and when either player is defeated or leaves the match. Allying again does not restore it. Loading a saved game restores the grants that were in force when it was saved.

## What the ally can do

An ally who holds a grant selects the owner's objects as they select their own: by clicking, by dragging a selection box, with Shift, with Select Same Type, with the veterancy and health filters, and in team groups. A selection can mix their own objects with the owner's. The ally sees the action lines of the owner's objects they select.

The ally can give the owner's objects every order the cursors and keys offer for their own. This includes moving, attacking, guarding, deploying, scattering and stopping, entering transports, setting a structure's rally point, and infantry abilities such as capturing, healing, planting C4 and stealing vehicles.

These actions stay with the owner:

- production from the sidebar
- selling, repairing, and turning power on or off
- choosing a primary factory
- selling walls

A trigger that fires when one of the owner's objects is selected fires only when the owner selects it.

## Orders checked by every machine

Every machine checks a grant when an order runs. An order for another player's object from someone without a grant is ignored, as described in [network packet validation](/systems/network-packet-validation/#command-contents).
