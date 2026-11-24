---
key: PostScore
summary: The movie played after the score screen of a won campaign mission.
see_also: [PreMapSelect, SkipScore, Win, Intro]
when_omitted:
  kind: value
  value: "<none>"
---

```ini title="map file"
[Basic]
PostScore=GDI_M09B
PreMapSelect=GDI_M11
```

The movie plays after the score screen and before [`PreMapSelect`](/keys/premapselect/). It still plays when [`SkipScore`](/keys/skipscore/) hides the score screen.

[`Intro`](/keys/intro/) covers how a movie name is resolved and what happens to one that cannot be found.
