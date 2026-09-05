---
title: Pause the music on focus loss instead of restarting it
category: feature
release: 0.2.0
targets:
- type: system
  id: music
  effect: added
credit: [ZivDero]
---

When the game loses the input focus the whole mix pauses in place, and it resumes where it stopped when the focus returns. The playing score used to be stopped and started again from its beginning. Music streams through the audio engine from its file, so a score that fades out on a scene change fades over the same second and a half as before.
