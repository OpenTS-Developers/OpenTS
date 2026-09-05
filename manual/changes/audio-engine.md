---
title: Replace DirectSound with the OpenTS audio engine
category: internal
release: 0.2.0
targets: []
credit: [ZivDero]
---

Sound effects, speech and music now play through a mixer of OpenTS's own on top of the miniaudio device layer, instead of DirectSound buffers driven by a timer thread. Up to sixteen sound effects play at once where five did before, and a headphone or output device change no longer silences the game: playback moves to the new device by itself. The movie player still uses its own DirectSound buffer for now, without changing the shared output format.
