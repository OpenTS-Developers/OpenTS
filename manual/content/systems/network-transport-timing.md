---
title: Network transport timing
summary: Measures the round trip of each private link and paces the retries of its unacknowledged packets.
category: multiplayer-networking
keys: []
---

In a network game, each machine keeps one private connection to every other player. Each private connection measures its round trip and uses that measurement to decide when to resend a packet that has not been acknowledged, and when that packet has taken too long. [Network synchronization](/systems/network-synchronization/) owns how the measured round trips set the command delay.

## Measuring a link

A clean measurement is the round trip of a packet that was sent only once and then acknowledged. An acknowledgement for a resent packet does not count, because it could answer any of the copies. The first clean measurement sets the link's smoothed round trip. Each later one moves it one eighth of the way toward the new value.

A link without a clean measurement takes its first estimate from the first acknowledgement it receives, even one for a resent packet. Without this, a link slower than its first retry delay would always resend before the acknowledgement arrived and could never be measured. This provisional estimate paces retries, but the link still counts as unmeasured for network synchronization. The first clean measurement also replaces a provisional estimate instead of averaging with it.

Measurement starts over when every player is ready at the start of a game or after loading a multiplayer save, so time spent loading does not count.

A frame packet normally asks for an acknowledgement only when it carries commands, so a player who issues no commands would leave its links unmeasured. In a [compressed game](/systems/network-synchronization/) with more than one player, a frame packet therefore asks for an acknowledgement at least every 32 frames while any private link lacks a clean measurement. The first frame packet of the game asks for one. This lets a quiet player's links be measured before the first timing evaluation.

## Retry timing

A measured link's retry timeout is its smoothed round trip plus four times that round trip's variation, held between 100 and 4000 ms.

A packet's first retry waits for the retry timeout that was in effect when the packet was first sent. Each further retry of the same packet doubles the wait, up to the connection timeout.

A link that has no estimate yet uses the fixed retry interval and connection timeout that the game sets for the session, with the connection timeout held between 2 and 30 seconds. Its retries double in the same way.

## Connection timeout

A measured link's connection timeout is the larger of these two values, held between 2 and 30 seconds:

- eight times the smoothed round trip, plus 250 ms
- four times the current retry timeout

The second value lets a packet be sent three times before it times out: when first sent, one retry timeout later, and three retry timeouts later. This still holds after the retry timeout doubles, but not after a new measurement lowers it.

A private connection never drops a packet. Once a packet is older than the connection timeout, the debug log records that the connection has gone bad. The connection keeps resending the packet until it is acknowledged.

## Links slower than their retry timeout

If a link's latency rises above its retry timeout, every packet is resent before its acknowledgement arrives. No acknowledgement is then clean, and the link could never be measured again. To prevent this, resending doubles the link's retry timeout, up to the 4000 ms maximum. This applies only to a link that already has an estimate, clean or provisional.

The retry timeout doubles once for each round of resends, not once for each resent packet. A resend doubles it only if its packet was first sent under a retry timeout at least as long as the current one. Packets first sent before the latest doubling leave it unchanged.

The next clean measurement recomputes the retry timeout from the smoothed round trip and its variation. Until then, the smoothed round trip keeps its last measured value, and the doubled retry timeout paces retries.

## Global channel

The global channel carries lobby messages, in-game chat, and other messages outside the private frame traffic. Its packets are never measured, and they retry at a fixed interval that does not double. Unlike a private connection, the global channel drops a packet that reaches its timeout, so one unanswered packet cannot block the channel.
