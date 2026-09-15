---
key: Ice3Set
summary: Third of the three sixty-four-tile ice sets a snow theater provides.
see_also: [Ice1Set, Ice2Set, IceShoreSet]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

The set has the same shape as [`Ice1Set`](/keys/ice1set/), which covers the layout the three share, but not the same treatment. Ice growth thickens only tiles of the first two sets, so a stretch laid from this one never grows.

Its start plus sixty-four marks the end of the run the shore pass treats as ice. When [`IceShoreSet`](/keys/iceshoreset/) works out the land-side piece for a cell, a neighbor counts as ice when it falls anywhere between the start of the first set and that point. The pass assumes the three sets sit one after another in registration order, so a gap or a reordering between them either drops ice out of that window or sweeps unrelated tiles into it.

This is not the same span as the test that asks whether an individual cell is ice, which [`Ice2Set`](/keys/ice2set/) covers and which stops one set short of this one.
