---
title: Network transport timing
summary: Measures each private link and schedules retries without changing synchronized frame timing.
category: multiplayer-networking
keys: []
---

Each private connection keeps a smoothed round trip, its variation, and a retry
timeout. Only the acknowledgement of a first transmission measures the link.
Until one arrives, the first acknowledgement after a retry seeds a provisional
estimate, so a link slower than the initial retry delay is still measurable; the
first clean acknowledgement replaces the seed. In a compressed game a frame
packet asks for an acknowledgement at least every 32 frames while any link still
lacks a clean measurement, so a quiet player's links are measured before the
first timing evaluation.

The retry timeout stays within 100 to 4000 ms, and each repeated private
transmission doubles its wait up to the connection timeout. For a measured link
the connection timeout is eight times the smoothed round trip plus 250 ms, or
four times the current retry timeout, whichever is larger, bounded to 2 to 30
seconds. Four times the retry timeout leaves room for three transmissions before
a packet times out, even once that timeout has backed off.

A packet older than the connection timeout marks the connection bad, and the
connection goes on retrying it until it is acknowledged. Receive-queue cleanup
runs during those retries, so a recovered link has room to drain its backlog. An
unmeasured link uses the bounded legacy timing, and global lobby traffic keeps
its fixed cadence.

A retransmitting link also doubles its retry timeout, once for each round of
retransmissions: only a packet first sent under a timeout at least as long as
the current one proves that timeout too short. Without this, a link whose
latency has climbed above its timeout retransmits every packet before the
acknowledgement arrives, and every sample stays ambiguous. The next clean
acknowledgement recomputes the timeout from the measured latency; until then the
smoothed round trip keeps its last measured value while the doubled timeout
paces retries.
