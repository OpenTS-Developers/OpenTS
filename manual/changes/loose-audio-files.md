---
title: Load sound effects from loose files and any archive
category: fix
release: 0.2.0
targets:
- type: format
  id: aud
  effect: changed
credit: [ZivDero, CCHyper]
---

A sound effect's sample is now looked up through the file layer when it first plays. A loose file in the game directory plays, and so does a member of any mounted archive; a loose file replaces an archived one. Before, a sample had to be in an archive cached at startup. `.WAV`, `.OGG`, `.FLAC` and `.MP3` samples are accepted alongside `.AUD`.

CCHyper is credited for the Vinifera sound loading this follows, which accepts the same formats from loose files.
