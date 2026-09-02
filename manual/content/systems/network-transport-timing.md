---
title: Network transport timing
summary: Measures each private link and schedules retries without changing synchronized frame timing.
category: multiplayer-networking
keys: []
---

Each private connection maintains smoothed round trip, variation, and a retry
timeout. Acknowledgements of first transmissions are the measurements. Until a
link has one, its first acknowledgement seeds a provisional estimate even after
a retry, so a link slower than the initial retry delay becomes measurable; the
first clean acknowledgement replaces the seed. In a compressed game, a frame
packet requests an acknowledgement at least every 32 frames while any link
still lacks a clean measurement, so a quiet player's links are measured
before the first timing evaluations.

The retry timeout is limited to 100–4000 ms. Repeated private transmissions
double their wait up to the connection timeout. For a measured link, the
connection timeout is the larger of eight times the smoothed round trip plus
250 ms and four times the current retry timeout, bounded to 2–30 seconds.
This lets the first retry wait for the backed-off timeout while allowing at
least three transmissions before the connection timeout.

A packet older than the connection timeout marks the connection bad but keeps
retrying until acknowledged. Receive-queue cleanup continues during these
retries, freeing space for the backlog when the link recovers. With no
measurement, the bounded legacy timing is used. Global lobby traffic retains
its fixed cadence.

A link that is retransmitting also doubles the timeout it measures against, once
per retransmission proven against the current value. This keeps a link whose
latency has risen above its timeout measurable, because every packet would
otherwise be retransmitted before its acknowledgement arrived and no
unambiguous sample could be taken. The next clean acknowledgement recomputes the
timeout from the measured latency; until then the estimate keeps its last
measured value while pacing retries.
