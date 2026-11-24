---
title: Network packet validation
summary: Rejects malformed or misattributed multiplayer traffic before it changes synchronized state.
category: multiplayer-networking
keys: []
---

## Damaged packets

Every incoming datagram must fit the receive buffer and match its checksum. A packet between two players must also have a complete header, a known packet code, and a payload length that suits that code. A packet that fails any of these checks is dropped and changes nothing.

## Command packets

A packet of player commands is accepted whole or not at all. Every command in it is decoded before any of them is queued, so one malformed command drops the whole packet.

Each command packet opens with the sender's frame report, and every command after it belongs to the same player. That player must be the one assigned to the connection the packet arrived on. The packet's source address must exactly match that connection's address. A command packet from any other address is dropped and never changes a player's stored address.

The periodic keep-alive report, which carries the scenario checksum, must be the only thing in its packet.

Each frame report gives the sender's current frame: the frame the report is scheduled for, less the sender's [look-ahead](/systems/network-synchronization/). The packet is dropped if that frame is negative or more than 250 frames ahead of the receiving machine's frame.

## Command contents

Each command is checked again when it runs. A command that fails is ignored, and the other commands of that frame still run. The checks are:

- The sending player exists.
- A production order names an existing vehicle, infantry, aircraft, or structure type.
- A superweapon strike names one of the sender's superweapons.
- A mission order names a defined mission, or none.
- An alliance names an existing player.
- A shared control grant names an existing player.
- A game speed change names one of the seven game speeds.
- A change to the legacy latency setting stays within `0` to `3`.

In a network game, a command aimed at one object requires the sender to own that object when the command runs. This covers power on or off, repair, sell, setting a primary factory or a rally point, deploy, scatter, stop, move, attack, and other mission orders, and the selection that offers a tag's [Selected by player](/mapping/events/tevent-selected/) event. An ally who has a [shared control](/systems/shared-control/) grant from the owner may also send all of these except power, repair, sell, setting a primary factory, and that selection. A command to a missing or destroyed object does nothing. A captured object ignores commands from its former owner.

## Interface rules in network games

In a network game, every machine also applies the following rules from the sidebar and the cursors when a command runs. A command that breaks one is ignored.

Three commands that no copy of the game sends are always ignored: a change to the special game rules, an animation placed by a player, and the old processing-time report.

Placing a building on the map requires all of the following:

- The object waiting in the sender's factory is a building. Vehicles, infantry, and aircraft leave their factory and cannot be placed on a chosen cell.
- The site is inside the playable map.
- The site passes the [base adjacency](/systems/base-adjacency/) check, or it is a building that accepts the upgrade being placed.
- No cell of the foundation is under the sender's shroud, unless the building lays its own tile.

Other commands have these rules:

- Starting production requires the prerequisites, tech level, and side that the sidebar requires. Resuming a paused object of the same type is always allowed.
- Selling a building requires a building that the sell cursor accepts. It cannot have [`Unsellable=yes`](/keys/unsellable/), be under construction or being sold, or be a deployed vehicle. A firestorm wall can be sold only while its firestorm is off. A move order that packs a building up into a vehicle still works.
- Selling a vehicle or aircraft requires it to be docked at a building with [`UnitRepair=yes`](/keys/unitrepair/).
- Starting a repair requires a building the repair cursor accepts: a damaged building with [`Repairable=yes`](/keys/repairable/), or one carrying a limpet drone. Stopping a repair is always allowed.
- Turning a building's power on or off requires [`TogglePower=yes`](/keys/togglepower/) and a building that uses or needs power.
- Selling a wall requires the sender to own the wall, with its cell clear of the sender's shroud.
- A mission order gives one of the missions a player can order: move, queued move, attack, guard, area guard, enter, capture, sabotage, harvest, patrol, or unload. An order with no mission is also accepted.
- A mission order's target and destination objects must not be under the sender's shroud. Objects that belong to the sender or an ally are exempt.
- A strike from an EM pulse superweapon with [`Action=EMPulse`](/keys/action/#scope-superweapontype) must target a cell in range of the cannon that will fire it, which is the sender's operational pulse cannon nearest the target. A superweapon with `Action=EMPulseRange` still fires at any distance.
- Making or breaking an alliance requires a match that allows alliances and does not fix them, a sender who has not been defeated, and a human target other than the sender.
- Granting or withdrawing shared control requires the conditions on the [shared control](/systems/shared-control/#granting-and-withdrawing-control) page.

Some of these commands can fail for a player whose game is not modified, because the match changes between the click and the command running. For example, the anchor for a placement or a prerequisite for production can be destroyed in that time, or a target can move into an area the sender has never seen.

In a network game, a command that an unmodified game cannot send also shows every player the message "*player* sent an illegal command." This covers the three commands that no copy of the game sends, a check from the [command contents](#command-contents) list that fails, and these interface rules: placing a vehicle, infantry, or aircraft on a chosen cell, placing outside the playable map, toggling power on a building that does not allow it, selling another player's wall, giving a mission that players cannot order, and an alliance change the match does not allow. The message appears at most once per player for each broken rule in a match, and nobody is removed from the match. A command that fails because the match changed in the meantime is ignored without a message.

## Timing changes

A change to the synchronized timing is accepted only from the [master](/systems/out-of-sync-recovery/#the-master). In a compressed game, the player who is master when the match starts stays master while still playing. After that player leaves, the lowest seat still held takes over.

The game speed and the legacy latency setting are not limited to the master. Any player can change either one, and every machine applies the change if its value is in range.

A change to the full timing sets the frame rate, send period, and look-ahead together. It is accepted only in a compressed game, and only when all of these hold:

- The frame rate is `1` to `60`.
- The send period is `1` to `10`.
- The look-ahead is a whole multiple of the send period.
- The look-ahead is at most `250` frames.
- The look-ahead is at least three send periods, or at least `4` frames when the send period is `1`.

A change to the look-ahead alone must be at least `2` frames. In a compressed game it must also be a whole multiple of the current send period, from two send periods up to `250` frames.

## Session messages

During a match, game and player queries are accepted from any address.

The following messages must come from a player in the match:

- chat
- loading progress
- sign-off
- ready
- kick proposals
- movie-skip votes
- the host announcement
- the out-of-sync dialog's heartbeat
- the decision to continue
- requests to load a saved game

A message of any other kind is dropped.

For these messages, the game identifies the sender by address, in this order:

1. It looks for players whose stored IP address and port both match. One match identifies the sender, and two or more drop the message.
2. If none match, it looks for players with the same IP address and a stored port of `0`. One match identifies the sender, and two or more drop the message.

A message that matches no player either way is dropped.

Some of these messages have further rules:

- A kick proposal must name another player in the match, not the sender.
- A loading progress report must be `0` to `100`.
- The decision to continue out of sync is accepted only from the master.
- A request to load a saved game is accepted only from the master. It carries the number of a [numbered save](/formats/save-games/#numbered-multiplayer-saves) from `0` to `999`, so it can name only a numbered save file in the saved-games folder.
- A host announcement is adopted while no host has been announced, or when it comes from the player already announced as host.

## What the checks do not prevent

Checksums and address matching detect damaged packets and tell players apart. They do not prove who a player is.

## Player removal

When a machine loses its connection to a player, it sends a synchronized report of that player's departure. The player is removed once more than half of the other players still in the match have reported it. Observers and defeated players who are still connected count among those players, and a player who has already been removed does not. In a two-player game, one report is enough.

The reported player's own machine ignores a report from a player it is still connected to. A forged report in a two-player game therefore puts the match out of sync instead of removing the player. A kick through the [reconnect dialog](/systems/reconnect-dialog/) still needs a vote from every other player.
