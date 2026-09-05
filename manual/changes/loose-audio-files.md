---
title: Load sound effects from loose files and any archive
category: fix
release: 0.2.0
targets:
- type: format
  id: aud
  effect: changed
credit: [ZivDero]
---

A sound effect's sample is now looked up through the file layer when it first plays, so a loose file in the game directory and a member of any mounted archive both play, and a loose file replaces an archived one. Before, a sample had to be in an archive cached at startup. `.WAV`, `.OGG`, `.FLAC` and `.MP3` samples are accepted alongside `.AUD`.
