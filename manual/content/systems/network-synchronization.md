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

Every synchronized-event packet passes through bounded transport, connection,
and event decoders before it can change the simulation. Its envelope is the
leading `FRAMEINFO` or sole `FRAMESYNC` record that carries the sender and frame
information for the packet. The receiver checks that envelope, the declared
event sizes, the complete event stream, and the player identity of the
connection that delivered it. A malformed, truncated, oversized, or
misattributed packet is discarded as one packet; no events from it enter the
simulation queue.

The connection identity becomes the origin of every compressed event in that
packet. Object commands for power, archive targets, repair, primary factories,
missions, idle, deployment, scattering, and selling also require the resolved
object to belong to that origin when the event executes. A missing or destroyed
object remains a no-op; an object captured before execution is rejected.

Only the resolved session master may change timing. The master may remove any
other player. When the master is the player being removed, the first remaining
network-human house in house order becomes the removal authority. Every
machine recomputes that rule when the removal event executes, and a repeated
removal has no further effect.

Public game and player discovery still accepts queries from outside the
session. Once a match is running, chat, loading progress, sign-off, ready, and
kick-control packets require one unique player-list endpoint. Resolution first
looks for an exact IP address and port. If none exists, it accepts one same-IP
entry whose stored port is zero; duplicate exact or fallback matches are
rejected. The recorded player identity, rather than the name or voter claimed
by the packet, owns that action.

The packet checksum detects damaged bytes. It is not authentication or
encryption. Global sender resolution does not pin or update the matched roster
address, and a network game still assumes that its players and the path carrying
their traffic are trusted.

## Link measurement and retransmission

Each connection measures its own round-trip time. An acknowledgement measures
the link only when its packet was transmitted once, because an acknowledgement
after a retry cannot identify which transmission it answers. Lost packets use
progressively longer retry intervals, while a healthy connection keeps the
interval derived from its own measurements instead of inheriting the slowest
other link in the match.

## Match timing

Every player periodically reports two bounded measurements as one record: the
processing time of its simulation frames and an optional worst round-trip time
among its active connections. The deterministic session master combines fresh
reports, chooses one timing rung, and sends the resulting frame rate, send
period, and look-ahead as a synchronized event. Reports from other players can
influence that decision, but a timing event sent by any of them is ignored.

The match begins with commands sent every three frames and a nine-frame
look-ahead; a guest accepts a compressed start only when the host advertises
that same look-ahead. A worse measured path can move directly to a more
conservative rung. Returning toward a more responsive rung requires sustained
headroom and moves one rung at a time, so short spikes do not make the timing
oscillate. A decrease waits until commands scheduled with the previous
look-ahead have cleared that horizon. It activates on a frame aligned to both
send periods, uses a temporary look-ahead that preserves the next command
target, and then removes one new send period at each later send boundary until
it reaches the requested value. A replacement target rebases the remaining
transition.

A player without an initial round-trip sample receives a 512-frame grace
period. A missing or expired sample after one was established is conservative
immediately; a sample that never becomes available is conservative when that
grace expires. Process time expires with the same report, and incomplete
process data retains the last synchronized frame rate. Timing membership begins
from the seated roster. An authorized removal clears the departing player's
whole report, allowing the remaining links to determine later decisions.

If the master leaves, its successor inherits the authoritative timing target
and the saturated eight-change budget. The successor discards prior improvement
evidence and begins with a cooldown instead of restarting from the initial
timing state. Once that budget is exhausted, later measurements can still make
timing more conservative but cannot reduce its rung or look-ahead.

The latency-margin setting keeps its existing four steps. They apply one,
one-and-a-half, two, or three times the measured round trip before a rung is
chosen. The game-speed setting also keeps its existing frame-rate mapping,
including speed zero as 60 frames per second.

## Compatibility

Network events and recorded multiplayer commands include the timing reports.
All players must use the same OpenTS snapshot, and a recording should be played
by the snapshot that wrote it. There is no player-facing timing setting to
migrate.
