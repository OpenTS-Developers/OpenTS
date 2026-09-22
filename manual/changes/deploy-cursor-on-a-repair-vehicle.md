---
title: Keep the deploy cursor on a vehicle that repairs
category: fix
release: 0.2.0
targets:
- type: key
  id: DeploysInto
  effect: changed
credit: [ZivDero, Rampastring]
---

A vehicle whose weapon repairs its own side now shows the deploy cursor over itself, so clicking it deploys it. It used to show a plain select cursor. No shipped vehicle both repairs and deploys.
