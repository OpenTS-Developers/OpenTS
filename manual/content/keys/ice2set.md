---
key: Ice2Set
summary: Second of the three sixty-four-tile ice sets a snow theater provides.
see_also: [Ice1Set, Ice3Set]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

The set has the same shape and the same treatment as [`Ice1Set`](/keys/ice1set/), which covers the layout the three share.

It has one role the others do not. The test that asks whether a cell is ice runs from the start of the first set to sixty-four tiles past the start of *this* one. With the three sets registered in order it accepts the first two and stops before [`Ice3Set`](/keys/ice3set/). Placing this set anywhere other than directly after the first therefore moves the boundary of what counts as ice.
