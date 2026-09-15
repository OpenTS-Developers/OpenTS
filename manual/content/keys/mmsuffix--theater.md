---
key: MMSuffix
scope: theater
label: Theater marble madness extension
see_also: [Suffix, NonMarbleMadness]
when_omitted:
  kind: context-dependent
  note: "`MMT` for TEMPERATE and `MMS` for SNOW, which keep their original settings; empty for any other theater, which then makes no second attempt."
---

Where a tile's own artwork is missing and its set does not set [`NonMarbleMadness=0`](/keys/nonmarblemadness/), the loader tries the name again under this extension. It is how the marble madness tile artwork stands in for tiles a theater does not draw itself.

```ini title="rules.ini"
[DESERT]
MMSuffix=MMD    ; RVCLIF01.MMD, tried after RVCLIF01.DES
```

A theater naming no marble madness extension makes no second attempt, and a tile whose own artwork is absent stays imageless.
