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
- An animation names a defined animation type, or none, and its owner, if it has one, is an existing player.
- A game speed change names one of the seven game speeds.
- A change to the legacy latency setting stays within `0` to `3`.

In a network game, a command aimed at one object requires the sender to own that object when the command runs. This covers power on or off, repair, sell, setting a primary factory or a rally point, deploy, scatter, stop, and move, attack, and other mission orders. A command to a missing or destroyed object does nothing. A captured object ignores commands from its former owner.

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

When a player leaves, every remaining machine queues that player's removal as a synchronized command. The first copy to run removes the player, and later copies do nothing.

Because every machine sends it, the removal command carries no check on who sent it. A modified game can use it to remove any other player, and every machine carries out the removal. A kick through the [reconnect dialog](/systems/reconnect-dialog/) still needs a vote from every other player.
