---
title: Center Base finds any construction yard type
category: fix
release: 0.2.0
targets:
- type: command
  id: CenterBase
  effect: changed
credit: [ZivDero]
---

Center Base now looks for any structure whose type is listed in `BuildConst`, preferring the primary one, so a mod's second construction yard type is found. Only the type the base unit deploys into counted before, and a base built from another construction yard centred on an arbitrary structure instead.
