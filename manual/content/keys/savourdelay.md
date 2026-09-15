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

A house that has won, lost, or been condemned to blow up does not act on it at once. The value is turned into a countdown in minutes at fifteen frames a second, and the default is a little under two seconds. The win, the loss, or the destruction of everything the house owns waits until that countdown reaches zero. It buys the player a moment to watch the shot that settled the match.

The countdown is set on the frame the fate is decided, and covers all three fates from the same figure. Two things bypass it. A trigger action can declare a win or a loss in a silent form that never sets the countdown. And a house that already holds a fate ignores a further flag for any other, so the first decision keeps its own delay. Flagging a house to lose is the exception: it cancels a pending win.

A network game rounds the delay up. It is first raised to at least the number of frames the session runs ahead of itself. It is then rounded so that it expires on a frame number that is a multiple of ten, which keeps every machine agreeing on when the game ends.
