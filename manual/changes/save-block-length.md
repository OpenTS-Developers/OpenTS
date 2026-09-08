---
title: State a partial save block's own length
category: fix
release: 0.2.0
targets:
- type: format
  id: save-games
  effect: changed
credit:
- mischa85
---

The last block of a saved game now states the number of bytes it expands to. It used to claim a full 64 KB however little the block held, and the loader worked around that by taking the length from the decompressor instead. A save written before this change loads unchanged.
