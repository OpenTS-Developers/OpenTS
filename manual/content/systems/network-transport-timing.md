---
title: Network transport timing
summary: Measures each private link and schedules retries without changing synchronized frame timing.
category: multiplayer-networking
keys: []
---

Each private connection maintains smoothed round trip, variation, and a retry
timeout. Only acknowledgements for first transmissions become samples, avoiding
ambiguous measurements after a retry.

The retry timeout is limited to 100–2000 ms. Repeated private transmissions
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
timeout from the measured latency. An estimate that has gone 8 seconds of
retransmissions without such an acknowledgement is treated as stale while still
pacing retries.
