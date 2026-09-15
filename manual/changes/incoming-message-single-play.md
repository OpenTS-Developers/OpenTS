---
title: Play the incoming message sound once per received line
category: fix
release: 0.2.0
targets:
- type: key
  id: IncomingMessage
  effect: changed
credit: [ZivDero]
---

A chat line from another player played `IncomingMessage`, in `[AudioVisual]` of `rules.ini`, twice: once as the line was added and again by the network handler. It now plays once, as every other line does.
