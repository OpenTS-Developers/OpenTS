---
title: Adapt private network retries
category: performance
release: 0.2.0
targets: []
credit:
- ZivDero
---

Each private connection now estimates its own round trip and backs off repeated
transmissions. Lobby traffic keeps its fixed retry cadence. Packet layouts,
event IDs, and configuration remain unchanged.
