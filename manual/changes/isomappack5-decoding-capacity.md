---
title: Size IsoMapPack5 decoding from its terrain payload
category: fix
release: 0.1.0
targets:
- type: format
  id: scenario-terrain
  effect: changed
credit: [ZivDero]
---

IsoMapPack5 no longer decodes through a 512,000-byte display surface. Its temporary storage now follows the maximum terrain-record payload and LZO block dimensions, allowing detailed maps whose valid compressed terrain passes the old display-sized limit. A stream larger than the format-derived bound is written to the debug log, and the scenario refuses to load with its map data reported as damaged. It used to be truncated silently into partial terrain.
