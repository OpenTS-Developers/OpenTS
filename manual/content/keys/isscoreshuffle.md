---
key: IsScoreShuffle
summary: Picks the next music track at random rather than taking the list in order.
see_also: [IsScoreRepeat, ScoreVolume]
when_omitted:
  kind: value
  value: "yes"
---

`IsScoreShuffle=yes` picks each next music track at random from the tracks the game currently allows. It picks the track that just ended only when no other track is allowed, and plays nothing when no track is allowed at all. With shuffle off, the game plays the allowed tracks in list order, starting after the track that just ended and wrapping around at the end.

[Choosing the next track](/systems/music/#choosing-the-next-track) lists what makes a track allowed.

While [`IsScoreRepeat`](/keys/isscorerepeat/) is on, a track that ends plays again, so shuffle chooses only a track that starts after silence. That page also covers how the sound options dialog keeps the two apart.
