---
key: OneTimeOnly
summary: Whether winning this mission ends the session instead of advancing the campaign.
see_also: [EndOfGame, SkipMapSelect, NextScenario]
when_omitted:
  kind: value
  value: "no"
---

```ini title="map file"
[Basic]
OneTimeOnly=yes
```

The setting is read after the score screen and both after-mission movies have been shown. When it is set, the game is shut down there and the player is returned to the main menu: no map selection, no campaign advance, and no closing movie.

:::caution[It is tested before the end-of-game handling]
A mission that sets both this and [`EndOfGame`](/keys/endofgame/) takes this path, so the campaign's closing movie and the credits are never reached.
:::
