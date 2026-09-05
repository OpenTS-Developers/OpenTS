---
title: Adapt private network retries
category: performance
release: 0.2.0
targets: []
credit:
- ZivDero
---

Each private connection now estimates its own round trip and backs off repeated
transmissions. The retry timeout backs off with them, so a link whose latency
rises above it stays measurable instead of retransmitting every packet. A link
slower than the initial retry delay is still measured. Timed-out packets keep
retrying, and receive queues keep freeing space so a recovered link can drain
its backlog. Lobby traffic keeps its fixed retry cadence. Packet layouts,
event IDs, and configuration remain unchanged.
