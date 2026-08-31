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
latency with a 2-second minimum and 30-second ceiling. With no measurement, the
bounded legacy timing is used. Global lobby traffic retains its fixed cadence.
