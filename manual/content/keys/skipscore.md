---
key: SkipScore
summary: Whether a won campaign mission passes over the score screen.
see_also: [PostScore, EndOfGame, OneTimeOnly]
when_omitted:
  kind: value
  value: "no"
---

```ini title="map file"
[Basic]
SkipScore=yes
```

`SkipScore=yes` removes the score screen that follows a won campaign mission, together with its music track and hall of fame. Nothing else changes. The [`Win`](/keys/win/) movie still plays first, [`PostScore`](/keys/postscore/) and [`PreMapSelect`](/keys/premapselect/) still follow, and the campaign advances as it would have.

The key applies only to a campaign mission. A skirmish or network match shows its own [score screen](/systems/multiplayer-score-screen/), which this key does not affect. A client suppresses that one with [`SkipScoreScreen`](/formats/spawn-ini/#what-a-player-is-shown).
