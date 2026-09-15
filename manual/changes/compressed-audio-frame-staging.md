---
title: Decode compressed audio frames in isolation
category: fix
release: 0.2.0
targets:
- type: format
  id: aud
  effect: changed
credit:
- ZivDero
---

A sound effect that starts while the music track or another compressed sound is being refilled no longer corrupts that other stream. The other stream used to play as loud static for the rest of the sound or track. A compressed frame whose sizes do not fit the decoder now ends the sample there, the same as a frame with a bad marker.
