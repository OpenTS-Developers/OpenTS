---
key: NodHunterSeeker
summary: Seeds the second side's HunterSeeker.
see_also: [HunterSeeker, "system:superweapons"]
when_omitted:
  kind: value
  value: none
---

The value becomes the [`HunterSeeker`](/keys/hunterseeker/#scope-side) of the second side in the rules' [`[Sides]`](/formats/rules-registries/) list, as each rules file sets it. A `HunterSeeker=` in that side's own section of the same file overrides it. [`GDIHunterSeeker`](/keys/gdihunterseeker/) does the same for the first side, and nothing else reads this key.
