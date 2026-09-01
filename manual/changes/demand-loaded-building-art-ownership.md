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

A structure with `DemandLoad=yes` now detaches archive-owned art after rules or save loading,
then loads and releases its own copy on demand. Structure and construction shapes are also
released as the byte arrays the file loader allocated. The previous code could free shared
archive memory or use a mismatched scalar release, corrupting the heap during theater setup,
construction-art cleanup or shutdown.

`FreeBuildup=yes` now releases construction art only with `DemandLoadBuildup=yes`. Used alone,
it leaves archive art attached, so later structures retain construction, deconstruction,
sellability and technician conversion for nominal crew on destruction.

Demand-loaded animations and overlays likewise detach archive shapes after settings or save
loading and release only their on-demand copies. Ordinary overlays now build the deferred
`.SHP` filename from their Image ID instead of an uninitialized buffer.
