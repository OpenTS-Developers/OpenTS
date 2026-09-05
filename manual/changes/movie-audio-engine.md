---
title: Play movie sound through the audio engine
category: feature
release: 0.2.0
targets:
- type: format
  id: vqa
  effect: changed
- type: key
  id: SoundLatency
  effect: changed
credit: [ZivDero]
---

A movie's sound track now plays through the audio engine instead of a DirectSound buffer of its own, so the game no longer links DirectSound at all. The picture follows the sound as before, timed from what the mixer has taken of the track and what the output device still holds, and it keeps running on wall time while the sound stands still. `SoundLatency` is still read and saved but has no effect.
