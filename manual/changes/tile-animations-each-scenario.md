---
title: Attach the specified tile animation in every scenario in a theater
category: fix
release: 0.3.0
targets:
- type: format
  id: theater-control
  effect: changed
credit:
- ZivDero
---

A scenario started in the theater already loaded attached each tile animation by its position among the previous scenario's animations. When a `Tile<NN>Anim` was set to an animation that the rules' `[Animations]` list leaves out, such a scenario could put a different animation on the tile or use an animation that no longer existed. The theater control file is now read for every scenario, so each `Tile<NN>Anim` is looked up among that scenario's animations. The stock files list every tile animation in `[Animations]` and were not affected.
