---
title: Network synchronization
summary: Adapts synchronized command delay to measured link and processing conditions.
category: multiplayer-networking
keys: []
---

A network game tags every command with the simulation frame on which each
machine executes it. Look-ahead gives a command time to arrive, and the send
period sets how many frames go into one packet.
[Network packet validation](/systems/network-packet-validation/) owns what a
packet must satisfy;
[Network transport timing](/systems/network-transport-timing/) owns each link's
round trip and retries.

## Adaptive policy

A compressed game, which packs a run of frames into one packet, begins at
`2/6`: a two-frame send period and six frames of look-ahead. Each player
reports its process time and its longest wait for the other players after 32
and 64 frames, then every 128 frames, and adds its worst local round trip once
it has one. A player leaves the round trip out while any of its links lacks a
[clean measurement](/systems/network-transport-timing/), and a link that is
retransmitting keeps reporting its last measurement. The master, the seat every
machine names the same way, evaluates at 64 and 128 frames, then every 256
frames.

A report carries process time and round trip together and expires after 512
frames. A player whose round trip never arrives within that time forces the
widest target, `10/250`. If a player that has been measured lets its report
expire or leaves the round trip out, the timing holds, though fresh reports
from other players can still worsen it. While any process report is stale, the
frame rate keeps its last synchronized value. The census starts from the
initial synchronized roster, and an executed removal clears that player's
report.

Headroom is the reported round trip raised by a quarter. The first complete
census selects its measured target with headroom, and a bootstrap still
incomplete at 128 frames falls back to `3/9`. A later worsening takes effect at
the evaluation that sees it. The first improvement needs three evaluations that
each have headroom and find no wait of 0.1 s or longer in any player's last two
report intervals. It also waits 256 frames after the last change, unless a
descent is already running. While headroom and the waiting limit both hold,
each following evaluation steps down one more rung. A worsening, or an
evaluation without headroom or with such a wait, restores the three-evaluation
requirement.

A decrease activates only once the old horizon has drained, on a frame aligned
to both send periods. At that frame the send period changes, the look-ahead
takes a temporary value, and each following boundary removes one new send
period until it reaches the target. An event already scheduled for a frame that
the new send period skips executes on the next send frame, identically on every
machine. A recording keeps it in that batch with its scheduled frame unchanged.
A target chosen later stages again from the timing then in force.

Losing a connection locally does not move the authority. The removal event
does, picking the first remaining human house, which inherits the target and
restarts the cooldown.

Frame pacing follows the desired frame rate alone.

## Player feedback

The disabled Connection slider shows the send-period rung in force, mirrored so
that rung 1 sits at its right end, and the label beside it names the tier and
the rung. Rungs 1 and 2 are Fast, 3 to 5 Normal, 6 to 8 Poor, and 9 and 10 Bad;
a look-ahead longer than its rung allows also reads as Bad. The message list
announces a change of target tier, which can arrive before a staged decrease
takes effect. The Speed slider sets game speed.

## Compatibility

`NETWORK_REPORT` is a new network event and appears in multiplayer recordings,
so every player needs the same OpenTS snapshot and a recording should be played
by the snapshot that wrote it. Existing event IDs are unchanged. The policy
reads the measured round trip directly; `LATENCYFUDGE` keeps its event ID and
its session field for replay compatibility, but nothing emits it any more and
the policy does not read it.
