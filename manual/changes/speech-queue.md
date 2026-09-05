---
title: Queue EVA lines instead of dropping them
category: feature
release: 0.2.0
targets:
- type: system
  id: eva-speech
  effect: added
credit: [ZivDero]
---

EVA lines now wait in a queue of up to eight and play one after another, half a second apart, with mission accomplished and mission failed ahead of the rest. A burst of different events therefore produces every line in order, where the game used to hold one pending line and drop any further request until it played. Speech streams from its file rather than being read into a fixed buffer first.
