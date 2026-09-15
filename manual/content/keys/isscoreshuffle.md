---
key: IsScoreShuffle
summary: Picks the next score at random rather than taking the list in order.
see_also: [IsScoreRepeat, ScoreVolume]
when_omitted:
  kind: value
  value: "no"
---

When a score ends and a successor is wanted, the flag decides how one is found. With the flag set, the game draws a score at random from the whole list. It draws again until the pick is one the game currently allows and is not the score that just finished, so the same score never plays twice in a row. A thousand fruitless draws fall back to the first score in the list. With the flag clear, the game walks the list forward from the score that just finished, wrapping around at the end.

The choice is only reached for a score that is not repeating. [`IsScoreRepeat`](/keys/isscorerepeat/) covers that, and the interlock the sound options dialog puts between the two.
