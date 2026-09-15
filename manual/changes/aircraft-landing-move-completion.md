---
title: Finish aircraft moves after landing completes
category: fix
release: 0.1.0
targets:
- type: system
  id: aircraft-operations
  effect: changed
credit: [ZivDero, hifi]
---

An aircraft following the normal move path now remains in its approach or landing state while its movement controller is still moving, even after it enters the destination cell. The move ends only after movement stops. When the destination becomes unsuitable, the aircraft follows the existing landing-zone recovery path.
