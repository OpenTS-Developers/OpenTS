---
key: IsScoreRepeat
summary: Repeats the playing score instead of moving on to the next one.
see_also: [IsScoreShuffle, ScoreVolume]
when_omitted:
  kind: value
  value: "no"
---

With the flag set, the score that has just been started queues up again as its own successor, so it plays until something else interrupts it. A score whose theme entry sets [`Repeat=`](/keys/repeat/) does this regardless of the flag.

The flag is also what suppresses the choice of a next score: while it is set, neither the sequential order nor the shuffled pick of [`IsScoreShuffle`](/keys/isscoreshuffle/) is read.

The sound options dialog shows the two as check boxes that turn each other off, so the dialog can never set both. A file that sets both is read as written, and repeating wins because the next score is never chosen. Leaving the options screen writes both flags back to `sun.ini`.
