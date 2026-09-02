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
slower than the initial retry delay is still measured, and a packet that
outlives the connection timeout keeps retransmitting instead of blocking the
link. Lobby traffic keeps its fixed retry cadence. Packet layouts, event IDs,
and configuration remain unchanged.
