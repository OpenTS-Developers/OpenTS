---
title: Music
summary: Scores stream from their files one at a time, pause with the rest of the mix when the game loses focus, and fade over a second and a half when a change of scene queues another.
category: audio-speech
keys: [ScoreVolume, IsScoreRepeat, IsScoreShuffle]
---

One score plays at a time, streamed from its `.AUD` through the [file layer](/formats/aud/) a block at a time, so nothing of it is held in memory between plays. [THEME.INI](/formats/theme-ini/) lists the scores and says which may be picked for ordinary play, which repeat, and which side may hear them.

## Changing scores

A scene that queues another score fades the playing one over a second and a half and starts the next once the fade is over; asking outright for a score cuts the playing one. When a score ends by itself the next is picked from the allowed list, at random when shuffle is on and in order otherwise, and a score marked to repeat, or every score while repeat is on, is queued again as it starts. A score whose file cannot be opened is left out of the pick and produces nothing when asked for outright.

## Focus and volume

When the game loses the input focus the whole mix pauses in place, the score with it, and resumes where it stopped when the focus returns. The music volume setting takes effect on the playing score. At zero the score is not started at all, and a score already playing keeps going silently until it ends.

A scenario's end stops the score and closes its file before the archives it came from are released.
