---
title: Network transport timing
summary: Measures each private link and schedules retries without changing synchronized frame timing.
category: multiplayer-networking
keys: []
---

Each private connection maintains smoothed round trip, variation, and a retry
timeout. Acknowledgements of first transmissions are the measurements. Until a
link has one, its first acknowledgement seeds a provisional estimate even after
a retry: the time since the packet's last transmission, paced no faster than the
retry delay that went unanswered. A link slower than the initial retry delay
therefore becomes measurable, and a peer that was still loading does not
inflate the seed. The first clean acknowledgement replaces it.

The retry timeout is limited to 100–4000 ms. Repeated private transmissions
double their wait up to the connection timeout; that timeout follows measured
latency with a 2-second minimum and 30-second ceiling. A packet's first retry
waits at most a quarter of that timeout, so every packet is sent at least three
times before it. A packet older than the timeout marks the connection bad but is
still retransmitted at the capped wait until it is acknowledged, so a link that
recovers drains its backlog. With no measurement, the bounded legacy timing is
used. Global lobby traffic retains its fixed cadence.

A link that is retransmitting also doubles the timeout it measures against, once
per retransmission proven against the current value. This keeps a link whose
latency has risen above its timeout measurable, because every packet would
otherwise be retransmitted before its acknowledgement arrived and no
unambiguous sample could be taken. The next clean acknowledgement recomputes the
timeout from the measured latency; until then the estimate keeps its last
measured value while pacing retries.
