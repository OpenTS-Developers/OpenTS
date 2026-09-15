---
key: Action
scope: scenarios
label: Scenario movie
see_also: [Intro, Brief, Theme, PostScore]
when_omitted:
  kind: value
  value: "<none>"
---

```ini title="map file"
[Basic]
Action=NOD_M04
Theme=APPROACH
```

The movie is the last thing between the briefing and the mission itself: it runs after the dropship loadout screen, if there is one, and only when the mission was started with its briefing. [`Theme`](/keys/theme/) is handed to it and queued as the movie opens.

With no movie named, the scenario's theme is started outright at the same point instead. A restart skips the movie but not the test that follows it. That test asks whether a movie was named, not whether one played, so a mission that names an action movie starts no score of its own when it is restarted. The name alone keeps the theme from starting. [`Theme`](/keys/theme/) covers what becomes of the score once the movie ends, and [`Intro`](/keys/intro/) covers how a movie name is resolved and what happens to one that cannot be found.
