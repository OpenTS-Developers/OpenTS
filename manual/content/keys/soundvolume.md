---
key: SoundVolume
summary: The volume of sound effects, as a fraction from 0 to 1.
see_also: [VoiceVolume, ScoreVolume, SoundLatency]
when_omitted:
  kind: value
  value: ".7"
---

The fraction is the level of the sound effect group in the mixer, applied on top of each sound's own volume rating and whatever volume the caller asked for, and it takes effect on sounds already playing; a sound effect is not started at all while the fraction is zero or below. The same fraction is the level of the movie group, so it sets the volume of a movie's audio track, and the menu, briefing and score screen sounds are sound effects and follow it too.

The read holds the fraction to `1` at the top but not at the bottom, so a negative figure is stored as written. Nothing else clamps it, and the sounds it gates are simply never heard.

The sound options dialog offers the fraction as a ten-step slider, and leaving the options screen behind it writes the setting back to `sun.ini`.
