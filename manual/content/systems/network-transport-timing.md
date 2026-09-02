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
first clean acknowledgement replaces the seed.

The retry timeout is limited to 100–4000 ms. Repeated private transmissions
double their wait up to the connection timeout; that timeout follows measured
latency with a 2-second minimum and 30-second ceiling. A packet older than that
timeout marks the connection bad but is still retransmitted at the capped wait
until it is acknowledged, so a link that recovers drains its backlog. With no
measurement, the bounded legacy timing is used. Global lobby traffic retains its
fixed cadence.

A link that is retransmitting also doubles the timeout it measures against, once
per retransmission proven against the current value. This keeps a link whose
latency has risen above its timeout measurable, because every packet would
otherwise be retransmitted before its acknowledgement arrived and no
unambiguous sample could be taken. The next clean acknowledgement recomputes the
timeout from the measured latency; until then the estimate keeps its last
measured value while pacing retries.
