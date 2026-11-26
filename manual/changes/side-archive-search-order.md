---
title: Search a side's archives before the game's own archives
category: feature
release: 0.2.0
breaking: true
migration:
- If a mod replaces a file that a side archive also holds by putting its copy in one of the game's own archives, such as `CONQUER.MIX` or `LOCAL.MIX`, move that copy into an `ECACHE` archive. Otherwise the side archive's copy is used during a match.
targets:
- type: format
  id: mix
  effect: changed
- type: system
  id: ui-files
  effect: changed
credit: [ZivDero]
---

During a match, the player's side archives are now searched right after the patch and expansion archives, so a side's copy of a file overrides the copy in the game's own archives and the theater archives. Previously they were searched after every archive mounted at startup. When the match ends, they move behind the other archives again.
