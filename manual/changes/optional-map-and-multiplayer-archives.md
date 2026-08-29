---
title: Stop requiring the map and multiplayer archives
category: fix
release: 0.2.0
targets:
- type: format
  id: mix
  effect: changed
credit:
- ZivDero
---

The game no longer refuses to start when no `MAPS*.MIX` and no `MULTI.MIX` are there. A
deployment that keeps its maps loose, or holds the multiplayer content in archives of its
own, now reaches the game rather than exiting during startup with nothing said. Both
archives are still mounted wherever they are found, and the startup log now names
`MULTI.MIX` only when it was mounted.

`TIBSUN.MIX` and `LOCAL.MIX` were guarded by a test that could never fail, so a missing one
of those has always been passed over rather than refused. Nothing about them changes: both
are mounted when present, exactly as before.

`CACHE.MIX` is still required, as are `CONQUER.MIX`, `SOUNDS.MIX`, `SCORES.MIX` and a movie
archive.
