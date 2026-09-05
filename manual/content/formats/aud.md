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

A sound effect names its samples in [SOUND.INI](/formats/sound-ini/) and looks each one up the first time it plays, through the ordinary file layer. A loose file in the game directory and a member of any mounted archive both serve, and the loose file wins; [MIX archives](/formats/mix/) covers which archives are mounted. Before `.AUD` the engine tries `.WAV`, `.OGG`, `.FLAC` and `.MP3` under the same name. A decoded sample is kept in memory while the sound plays and afterwards for as long as the memory is not needed for another, so a sound plays from memory from its second use on. A sound whose sample is not found is still registered under its ID and never plays.

Music is streamed. A music track's `.AUD` is opened by name through the same file layer each time it plays and read a block at a time, so nothing of it is held in memory between plays. A track whose file cannot be opened is left out of shuffled and sequential play, and produces nothing when it is asked for directly.

## When either one is silent

A sound effect is played only while all of these hold, tested in this order:

- the volume asked for is above zero, which for an ordinary sound effect means the player's sound effect setting as well;
- the game was not started quiet;
- an audio device is available;
- at least one of its samples is found.

Music is played only while all of these hold, tested in this order:

- an audio device is available;
- the game was not started quiet;
- the music volume is above zero.

## What the reader takes from the file

The start of the file supplies the playback rate, the size of the data, the size it uncompresses to, a set of flags and a compression code. Two flags are read: one marks the sample as stereo, the other marks it as sixteen bit. Any rate, either bit depth, and mono or stereo all play; a rate above 20000 and below 24000 hertz is played as 22050 whatever the file asked for.

Three compression codes are decoded: uncompressed data, the earlier Westwood delta compression, and the frame compression the shipped files carry. A sample with any other code is refused and does not play. Within a frame-compressed sample each frame is preceded by its compressed size, its uncompressed size and a fixed marker, and a frame whose marker does not match, or whose sizes exceed the decoder's limits, ends the sample there instead of failing the play. A sample is decoded once and kept in memory in its decoded form for as long as it is in use or the memory is not needed for another.
