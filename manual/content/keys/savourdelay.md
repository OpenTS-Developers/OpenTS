---
key: SavourDelay
summary: Delay, in minutes, between a house's fate being decided and it being carried out.
when_omitted:
  kind: value
  value: ".03"
---

```ini title="rules.ini"
[AudioVisual]
SavourDelay=.03
```

`SavourDelay` sets how long a house waits between being flagged to win, lose or blow up and that result taking effect. The value is in minutes of game time: it is multiplied by 900 frames and truncated. Left unset, the delay is 27 game frames, just under two seconds at 15 frames a second. `SavourDelay=.03` written in the file gives 26 frames, because a value read from the file is stored at slightly lower precision.

When the countdown ends, the result takes effect: the win, the loss, or the destruction of everything the house owns. In a campaign mission a win can wait longer, as [The win sequence](/systems/campaign-progression/#the-win-sequence) describes.

The countdown starts when the fate is decided, with these exceptions:

- The [Announce Win](/mapping/actions/taction-announce-win/) and [Announce Lose](/mapping/actions/taction-announce-lose/) trigger actions do not start it.
- A house already flagged for a fate ignores a later flag to win or to blow up, and its first countdown continues.
- A flag to lose cancels a pending win and starts a new countdown. It is ignored when the house is already flagged to lose or to blow up.

In a multiplayer game other than skirmish, the countdown is extended to end on a frame number divisible by ten.
