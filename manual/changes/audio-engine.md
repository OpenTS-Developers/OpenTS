---
title: Replace DirectSound with the OpenTS audio engine
category: internal
release: 0.2.0
targets: []
credit: [ZivDero, CCHyper]
---

Sound effects, speech and music now play through a mixer OpenTS provides, built on the miniaudio device layer. It replaces the DirectSound buffers a timer thread used to drive. Up to sixteen sound effects play at once, where five did before. Changing the headphone or output device no longer silences the game, because playback moves to the new device by itself.

CCHyper is credited for the Vinifera audio system, which put the game on miniaudio first and whose loudness curve and movie clock this engine keeps.
