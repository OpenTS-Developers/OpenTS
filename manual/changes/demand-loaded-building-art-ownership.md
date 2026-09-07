---
title: Keep demand-loaded artwork under its owner's lifetime
category: fix
release: 0.2.0
targets:
- type: key
  id: DemandLoad
  scope: buildingtype
  effect: changed
- type: key
  id: DemandLoad
  scope: animtype
  effect: changed
- type: key
  id: DemandLoad
  scope: overlaytype
  effect: changed
- type: key
  id: DemandLoadBuildup
  effect: changed
- type: key
  id: FreeBuildup
  effect: changed
credit: [Krisztiaan, ZivDero]
---

A structure with `DemandLoad=yes` could corrupt the heap during theater setup, construction-art cleanup or shutdown, because it freed shared archive memory or released its own art through a mismatched call. It now detaches archive-owned art after rules or save loading and loads and releases only its own copy on demand; demand-loaded animations and overlays do the same.

`FreeBuildup=yes` releases construction art only alongside `DemandLoadBuildup=yes`. Used alone it leaves archive art attached, so later structures keep their construction and deconstruction sequences, their sellability, and the technicians a nominal crew leaves on destruction.

An ordinary overlay's deferred `.SHP` name is built from its Image ID instead of an uninitialized buffer.
