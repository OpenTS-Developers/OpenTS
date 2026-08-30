---
title: Network synchronization
summary: Keeps every machine on the same simulation frame while adapting command delay and retransmission timing to the measured links between the players.
category: multiplayer-networking
keys: []
---

Network games exchange commands rather than copies of the game state. Each
command names the simulation frame on which every machine executes it, and a
machine waits when advancing would carry it too far beyond a player whose
commands have not arrived. The look-ahead distance is therefore both the time
available for delivery and the delay before a player's command takes effect.

## Packet admission

Every packet passes through bounded transport, connection, and event decoders
before it can change the simulation. The receiver checks the packet envelope,
the declared event sizes, the complete event stream, and the player identity of
the connection that delivered it. A malformed, truncated, oversized, or
misattributed packet is discarded as one packet; no events from it enter the
simulation queue.

Public game and player discovery still accepts queries from outside the
session. Once a match is running, chat, loading progress, sign-off, ready, and
kick-control packets are accepted only from an address recorded in the player
list. The recorded player identity, rather than the name or voter claimed by
the packet, owns that action.

The packet checksum detects damaged bytes. It is not authentication or
encryption, and a network game still assumes that its players and the network
path carrying their traffic are trusted.

## Link measurement and retransmission

Each connection measures its own round-trip time. An acknowledgement measures
the link only when its packet was transmitted once, because an acknowledgement
after a retry cannot identify which transmission it answers. Lost packets use
progressively longer retry intervals, while a healthy connection keeps the
interval derived from its own measurements instead of inheriting the slowest
other link in the match.

## Match timing

Every player periodically reports two bounded measurements: the processing
time of its simulation frames and the worst round-trip time among its own
connections. The deterministic session master combines the reports, chooses
one timing rung, and sends the resulting frame rate, send period, and
look-ahead as a synchronized event. Reports from other players can influence
that decision, but a timing event sent by any of them is ignored.

The match begins with commands sent every three frames and a nine-frame
look-ahead. A worse measured path can move directly to a more conservative
rung. Returning toward a more responsive rung requires sustained headroom and
moves one rung at a time, so short spikes do not make the timing oscillate. A
decrease waits until commands scheduled with the previous look-ahead have
cleared that horizon.

A connected player whose established report expires is treated
conservatively. Removing that player removes its report as well, allowing the
remaining links to determine later timing decisions.

The latency-margin setting keeps its existing four steps. They apply one,
one-and-a-half, two, or three times the measured round trip before a rung is
chosen. The game-speed setting also keeps its existing frame-rate mapping,
including speed zero as 60 frames per second.

## Compatibility

Network events and recorded multiplayer commands include the timing reports.
All players must use the same OpenTS snapshot, and a recording should be played
by the snapshot that wrote it. There is no player-facing timing setting to
migrate.
