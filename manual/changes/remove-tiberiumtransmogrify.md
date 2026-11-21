---
title: Stop reading TiberiumTransmogrify
category: feature
release: 0.3.0
targets:
- type: key
  id: TiberiumTransmogrify
  effect: removed
credit:
- ZivDero
---

`TiberiumTransmogrify=` under `[General]` in `rules.ini` is no longer read. It never had an effect: the engine stored the number and nothing used it. A `rules.ini` that still sets it loads and plays as before.
