---
title: Audio engine
summary: A fixed pool of voices rendered on the device thread, fed by a lock-free command ring from the game thread and by a feeder thread that fills streams and recovers a lost device.
category: architecture
source_files:
  - code/audio/audiodefs.hh
  - code/audio/audiohandle.h
  - code/audio/audioring.h
  - code/audio/audiolevel.cpp
  - code/audio/audiodecode.cpp
  - code/audio/audiosample.cpp
  - code/audio/audiomixer.cpp
  - code/audio/audiostream.cpp
  - code/audio/audiodevice.h
  - code/audio/audiodevice_ma.cpp
  - code/audio/audioevent.cpp
  - code/audio/audioengine.cpp
  - code/audio/audiomovie.cpp
  - code/sndtype.cpp
  - code/voc.cpp
  - code/vox.cpp
  - code/theme.cpp
---

The engine lives under `code/audio/` and is owned by one global, `AudioEngine`. Three threads touch it. The game thread calls every public member; the device thread runs the mixer's render callback; the feeder thread fills stream rings and recovers a lost device. The render path never allocates, locks, logs, or opens a file.

## Layers

The device layer is an interface with one implementation over miniaudio, which OpenTS vendors for the device, the resampler and the WAV, OGG, FLAC and MP3 decoders only; no miniaudio engine or node graph is used, so another library could stand in behind the same interface. The device is opened as 48 kHz stereo float with three periods of 10 ms, whatever the hardware's own rate, so a reroute to another device cannot change the mixer's rate.

The mixer holds sixty-four voices. Each voice plays either a sequence of decoded clips, attack, body loop and decay, or a stream ring, resampling from the source rate and pitch to 48 kHz, applying ramped voice, group, duck and master levels through one perceptual loudness curve, and summing into the bus, which is soft-clipped above 0.9. Every change reaches the mixer as a command in a single-producer, single-consumer ring; a command names a voice by slot and generation, so a late command for a voice that has since been reused is dropped. Voice states move from allocated through playing, paused and stopping to done, and the game thread reclaims a voice only after it is done.

The sample cache keeps decoded PCM keyed by name or by the address and hash of an in-memory AUD, pinned while any event uses it and evicted least-recently-used within sixty-four megabytes; a single sample is capped at eight megabytes.

Streams are rings the mixer reads and a producer writes. File streams decode an AUD chunk at a time, or any other format through miniaudio, on the feeder thread, five seconds ahead; the movie player pushes its PCM blocks into a ring of its own and reads the frames the mixer has consumed back as its clock. The feeder runs a plain periodic loop every 16 ms: it fills every attached stream, and while the device reports itself stopped it renders the mixer into the void at wall-clock rate, so voices finish and the movie clock keeps moving, retries the device every two seconds, and reopens it after ten.

The event pool turns a sound type into a sequence and hands it to a voice. It enforces each type's `Limit=`, keeps the effects budget of `Channels=` voices with priority stealing, drives delayed loops one cycle at a time, and answers the four-byte generational handles the game holds. Streams and raw in-memory samples are events too, so one handle type covers everything the game controls.

## Game layer

`VocClass` owns one sound type per SOUND.INI section and plays it as an event; the positional model, the placed-sound pool and the ambient table live beside it in `voc.cpp` and `ambient.cpp`. Speech queues lines and plays each as a stream; the theme plays a score as a stream; the option sliders are the group levels; the radar movie ducks the other groups. Focus loss flips one atomic flag that the render callback consumes, so it is safe from whichever thread delivers the window message.

## Invariants

The mixer writes voice state; the game reads it. A sequence is written by the game before the play command and never changed while the voice runs it. A stream ring is written by one thread and read by one thread. The random draws for pitch and loudness come from the non-critical generator, so audio never touches the simulation's random sequence. Every engine member is a no-op when no device could be opened.
