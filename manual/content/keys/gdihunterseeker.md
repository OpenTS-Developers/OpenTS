---
key: GDIHunterSeeker
summary: Seeds the first side's HunterSeeker.
see_also: [HunterSeeker, "system:superweapons"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[General]
GDIHunterSeeker=GHUNTER
```

The named UnitType becomes the first side's [`HunterSeeker`](/keys/hunterseeker/#scope-side) as each rules file that has the key sets it; a `HunterSeeker=` in that side's own section of the same file overrides it. It has no other effect.
