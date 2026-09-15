---
key: MultipleFactory
summary: The speed-up each factory past the first gives to production of its category.
when_omitted:
  kind: value
  value: "1"
---

A house that owns more than one factory able to produce the object's category has its build time multiplied by `1 / ((count - 1) * MultipleFactory)`. It counts the house's structures whose [`Factory=`](/keys/factory/) names that category, switched on or not. A value of zero or below skips the adjustment.

At the default the second factory therefore changes nothing, since one divided by one is one, and the third halves the time. A value below 1 makes a pair of factories slower than a single one.
