---
key: Intro
summary: The movie played first when a campaign mission is started with its briefing.
see_also: [Brief, Action, Win, Lose, PostScore, PreMapSelect]
when_omitted:
  kind: value
  value: "<none>"
---

```ini title="map file"
[Basic]
Intro=INTRO
Brief=GDI_M02
```

The value names an entry of the art file's `[Movies]` list rather than a file. The engine appends `.VQA` to the registered name when it goes looking for the film. `<none>`, an empty value, and any name the list does not contain all leave the setting as it was. Omitting the key does the same.

The movie runs once the scenario has been read and just before [`Brief`](/keys/brief/), and only when the mission was started fresh. A mission replayed after a loss or restarted from the menu passes over both. Outside a campaign the movie plays only when the [launch file](/formats/spawn-ini/) asks for movies through its `PlayMoviesInMultiplayer` switch. A registered name whose `.VQA` file is missing, or whose picture is both narrower than 320 and shorter than 200, is skipped in silence. The setting does not survive between missions: the engine returns every movie selection to none before each map is read, so a name the list does not contain affects only the mission that wrote it.

:::danger[A long registered movie name overruns a fixed buffer]
Every movie assignment is resolved by one shared routine, which copies the registered name and the `.VQA` suffix into a 20-byte buffer without checking the length. A `[Movies]` entry longer than fifteen characters writes past the end of that buffer, over whatever the linker placed after it. The names the game ships are at most eleven characters, so only a mod that registers longer ones can reach this.
:::
