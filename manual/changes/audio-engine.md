---
title: Replace DirectSound with the OpenTS audio engine
category: internal
release: 0.2.0
targets: []
credit: [ZivDero, CCHyper]
---

Sound effects, speech and music now play through a mixer of OpenTS's own on top of the miniaudio device layer, instead of DirectSound buffers driven by a timer thread. Up to sixteen sound effects play at once where five did before, and a headphone or output device change no longer silences the game: playback moves to the new device by itself.

CCHyper is credited for the Vinifera audio system, which put the game on miniaudio first and whose loudness curve and movie clock this engine keeps.
