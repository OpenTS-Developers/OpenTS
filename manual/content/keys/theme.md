---
key: Theme
summary: The score a mission opens with, queued at the point where its action movie plays.
see_also: [Action, Intro]
when_omitted:
  kind: value
  value: "No theme"
---

```ini title="map file"
[Basic]
Action=NOD_M04
Theme=APPROACH
```

The value names a score declared in [the theme control file](/formats/theme-ini/), and it is consulted at one moment: the point between the briefing and the mission where [`Action`](/keys/action/#scope-scenarios) would play.

A mission that names no action movie queues the score there, and does so whether or not the mission was started with its briefing, so a restarted mission gets its score back. A mission that does name one hands the score to the movie, which queues it as the movie opens. The score system is then asked for a fresh track as soon as the movie ends. That request is passed over while the queued score is still waiting its turn, and taken — fading the score out — once that score has begun.

Nothing returns to the setting afterwards. Once the queued score has played, normal track rotation chooses the next allowed score.

:::caution[A partial title can select a different score]
The name is matched against each score's own identifier first, ignoring case. If that fails, it is matched again as a substring of each score's displayed title, this time with case respected, and the first score whose title contains the text wins. A name that matches neither stores no score at all, which is the same as leaving the key out.
:::
