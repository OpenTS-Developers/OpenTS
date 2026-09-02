---
title: Network synchronization
summary: Adapts synchronized command delay to measured link and processing conditions.
category: multiplayer-networking
keys: []
---

Network games exchange commands tagged with the simulation frame on which every
machine executes them. Look-ahead gives those commands time to arrive, while the
send period controls how often compressed packets are emitted. Packet validation
is covered by [Network packet validation](/systems/network-packet-validation/);
per-link RTT and retry behavior belongs to
[Network transport timing](/systems/network-transport-timing/).

## Adaptive policy

Compressed matches begin at `2/6`: a two-frame send period and six-frame
look-ahead. Each player reports process time, its longest wait for other
players, and optional worst-local RTT after 32 and 64 frames, then every
128 frames. A player omits the RTT while any of its
links has no [clean measurement](/systems/network-transport-timing/); a link
keeps reporting its last measurement while it retransmits. The deterministic
master evaluates at 64 and 128 frames, then every 256 frames.

A report is one atomic process/RTT record and expires after 512 frames. A
player whose RTT never appears within that time selects the conservative
`10/250` target. An established player whose report expires or omits the RTT
holds the current timing, though fresh reports from other players can still
worsen it. Stale process data retains the last synchronized frame rate.
Membership comes from the initial synchronized roster, and accepted removal
clears that player's report.

The first complete census may select its measured target with 20% headroom.
Incomplete bootstrap falls back to `3/9` after 128 frames. Later worsening is
immediate. The first improvement needs three evaluations with 20% headroom and
no wait of 0.1 s or longer within any player's last two report intervals;
while both hold, each following evaluation steps one more rung. A worsening or
an evaluation without headroom or with such a wait restores the
three-evaluation requirement.

Timing decreases activate only after the old horizon drains on a frame aligned
to both send periods. They switch rate with temporary look-ahead, then remove
one new send period at each boundary. An event already scheduled for a frame
that the new send period skips executes on the next send frame, identically on
every machine. Replacement targets rebase this process; local connection
teardown does not transfer authority. Accepted removal selects the first
remaining human, which inherits the target and restarts the cooldown.

Frame pacing follows the desired frame rate alone. The inherited slowdown that
stretched every frame by up to 30 ms while a player's newest frame packet
looked a quarter of the look-ahead old is gone: at the adaptive send periods
that packet is always at least that old, so the slowdown ran on every frame
and held the game well under its frame rate on an idle link.

## Player feedback

The disabled Connection slider shows the effective send-period rung. Rungs 1–2
are Fast, 3–5 Normal, 6–8 Poor, and 9–10 Bad; extended look-ahead is also Bad.
The message list announces target-tier changes, which may precede a safely
staged improvement. The Speed slider continues to control game speed.

Adaptive timing uses measured RTT directly. The legacy `LATENCYFUDGE` event and
session field remain for replay compatibility, but the menu no longer emits it
and the adaptive policy does not consume it.

## Compatibility

`NETWORK_REPORT` extends network events and multiplayer recordings. All players
must use the same OpenTS snapshot, and recordings should be played by the
snapshot that wrote them. Existing event IDs retain their values.
