---
title: Music
summary: Scores stream one at a time, fade over a second and a half when another is queued, and pause with the rest of the mix when the game loses focus.
category: audio-speech
keys: [ScoreVolume, IsScoreRepeat, IsScoreShuffle]
---

One score plays at a time, streamed from its `.AUD` through the [file layer](/formats/aud/). [THEME.INI](/formats/theme-ini/) lists the scores and says which may be picked for ordinary play, which repeat, and which side may hear them.

A machine's own music settings are written together:

```ini title="sun.ini"
[Audio]
ScoreVolume=1.0
IsScoreShuffle=yes
IsScoreRepeat=no
```

## Changing scores

Queuing another score fades the playing one over a second and a half. The next starts once the fade is over, and asking for a score outright cuts the playing one. When a score ends by itself the next is picked from the allowed list, at random when shuffle is on and in order otherwise. A score marked to repeat is queued again as it starts. While repeat is on, every score is queued again that way. A score whose file cannot be opened is left out of the pick and produces nothing when asked for outright.

## Focus and volume

When the game loses the input focus the whole mix pauses in place, the score with it, and resumes where it stopped when the focus returns. The music volume setting takes effect on the playing score. At zero the score is not started at all, and a score already playing keeps going silently until it ends.

Starting a scenario stops the score and closes its file before the new scenario's archives are mounted.
