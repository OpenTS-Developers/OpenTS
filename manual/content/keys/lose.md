---
key: Lose
summary: The movie played when a campaign mission is lost.
see_also: [Win, Intro]
when_omitted:
  kind: value
  value: "<none>"
---

```ini title="map file"
[Basic]
Lose=KILLMECH
```

The movie is shown after the defeat announcement has finished speaking and before the offer to replay the mission. Accepting that offer restarts the scenario without its briefing, so [`Intro`](/keys/intro/), [`Brief`](/keys/brief/) and [`Action`](/keys/action/#scope-scenarios) do not play again. Multiplayer and skirmish games end their own way and reach the movie only when the launch file asked for movies, and then after their own score screen; [multiplayer movies](/systems/multiplayer-movies/) owns that.

[`Intro`](/keys/intro/) covers how a movie name is resolved and what happens to one that cannot be found.
