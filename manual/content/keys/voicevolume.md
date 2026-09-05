---
key: VoiceVolume
summary: The volume of EVA and taunt speech, as a fraction from 0 to 1.
see_also: [SoundVolume, ScoreVolume]
when_omitted:
  kind: value
  value: "1"
---

The fraction is the level of the speech group in the mixer, set as it is read and again whenever the slider moves, and it takes effect on the line that is speaking. Speech is refused outright while the fraction is zero or below, so nothing is queued and nothing is spoken. The fraction covers the EVA announcements and the multiplayer taunts; unit responses are sound effects and follow [`SoundVolume`](/keys/soundvolume/) instead.

The read holds the fraction to `1` at the top but not at the bottom, so a negative figure is stored as written and simply silences the speech.

The sound options dialog offers the fraction as a ten-step slider, and leaving the options screen behind it writes the setting back to `sun.ini`. Moving that slider plays a taunt back as feedback while a game is running, and a beep otherwise.
