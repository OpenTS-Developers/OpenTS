---
format_id: aud
title: AUD audio
summary: Stores sound effects, speech, and music consumed by the OpenTS audio layer.
kind: binary
extensions:
  - .AUD
role: audio
related:
  - { type: format, id: mix }
source_files:
  - code/audio/audiodecode.h
  - code/audio/audiodecode.cpp
  - code/audio/audiosample.cpp
  - code/audio/audiostream.cpp
  - code/voc.cpp
  - code/theme.cpp
---

Sound effects and music both play from `.AUD` files, but the two paths do not find those files the same way, and the difference decides where a modded sample has to be put.

## How a sample is found

A sound effect takes hold of its sample when the sound is registered and keeps that one pointer for the rest of the session. The pointer comes from an archive that was cached into memory, so the sample has to be a member of one: [MIX archives](/formats/mix/) covers what caching does, and a loose `.AUD` in the game directory is not reached by that path at all. Startup caches `CONQUER.MIX` and the shipped sound archives, and among the numbered expansion archives it caches the `ECACHE` set but not the `EXPAND` set. A sound whose sample is not found is still registered under its ID and simply never plays.

Music is streamed instead. A music track's `.AUD` is opened by name through the ordinary file layer each time it plays and read a block at a time, so a loose file works and nothing is held in memory between plays. A track whose file cannot be opened is left out of shuffled and sequential play, and produces nothing when it is asked for directly.

## When either one is silent

A sound effect is played only while all of these hold, tested in this order:

- the volume asked for is above zero, which for an ordinary sound effect means the player's sound effect setting as well;
- the sound took hold of a sample when it was registered;
- the game was not started quiet;
- an audio device is available.

Music is played only while all of these hold, tested in this order:

- an audio device is available;
- the game was not started quiet;
- the music volume is above zero.

The one term the two lists do not share is the sample test, and a silent session reaches that as well as the quiet one. The shipped sound archives are cached only when a device is present and the game was not started quiet, so a sound whose sample lives in one of those holds nothing at all in a silent session. A sample in `CONQUER.MIX` or in an `ECACHE` archive is still taken hold of, since those are cached whatever the audio state, and that sound is silent on the quiet test rather than for want of a sample.

## What the reader takes from the file

The start of the file supplies the playback rate, the size of the data, the size it uncompresses to, a set of flags and a compression code. Two flags are read: one marks the sample as stereo, the other marks it as sixteen bit. Any rate, either bit depth, and mono or stereo all play; a rate above 20000 and below 24000 hertz is played as 22050 whatever the file asked for.

Three compression codes are decoded: uncompressed data, the earlier Westwood delta compression, and the frame compression the shipped files carry. A sample with any other code is refused and does not play. Within a frame-compressed sample each frame is preceded by its compressed size, its uncompressed size and a fixed marker, and a frame whose marker does not match, or whose sizes exceed the decoder's limits, ends the sample there instead of failing the play. A sample is decoded once and kept in memory in its decoded form for as long as it is in use or the memory is not needed for another.
