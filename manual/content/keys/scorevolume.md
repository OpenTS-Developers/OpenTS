---
key: ScoreVolume
summary: The volume of the music, as a fraction from 0 to 1.
see_also: [SoundVolume, VoiceVolume, IsScoreRepeat, IsScoreShuffle]
when_omitted:
  kind: value
  value: ".5"
---

The fraction is the level of the music group in the mixer, set as it is read and again whenever the slider moves; a score already playing is turned to the new volume without being restarted. A score is not started at all while the resulting volume is zero or below — the request is remembered as pending instead, so raising the volume later starts the score that was waiting.

The read holds the fraction to `1` at the top but not at the bottom, so a negative figure is stored as written and leaves every score pending.

The sound options dialog offers the fraction as a ten-step slider, and leaving the options screen behind it writes the setting back to `sun.ini`. The credits screen raises a fraction of zero to `.4` for the length of its own theme and puts the stored figure back afterward.
