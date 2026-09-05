---
key: SoundLatency
summary: Former movie sound offset, in sixtieths of a second, that the engine reads and writes back but no longer uses.
no_effect: true
see_also: [SoundVolume, StretchMovies]
when_omitted:
  kind: value
  value: "9"
---

A movie keeps its video in step with its audio by asking how much of the sound has been heard and drawing to match. The audio engine answers that from what the mixer has taken of the sound track and what the output device still holds, so there is nothing left for a hand-set offset to correct. The figure once stood in for that device latency on emulated DirectSound drivers.

The setting has no control on any options screen. Saving the options still writes it back to `sun.ini` with the rest, so the file round-trips unchanged.
