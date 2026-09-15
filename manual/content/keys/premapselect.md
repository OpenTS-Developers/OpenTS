---
key: PreMapSelect
summary: The movie played on the way from a won campaign mission to the next map choice.
see_also: [PostScore, SkipMapSelect, OneTimeOnly, Intro]
when_omitted:
  kind: value
  value: "<none>"
---

```ini title="map file"
[Basic]
PreMapSelect=GDI_M11
```

The movie runs after [`PostScore`](/keys/postscore/) and before the campaign decides where to go next. It plays even when that decision skips the map selection screen or ends the game: [`OneTimeOnly`](/keys/onetimeonly/) and [`EndOfGame`](/keys/endofgame/) are both tested afterwards.

[`Intro`](/keys/intro/) covers how a movie name is resolved and what happens to one that cannot be found.
