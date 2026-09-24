---
key: MultipleFactory
summary: The build-time multiplier each factory past the first applies to production of its category.
when_omitted:
  kind: value
  value: "1"
---

A house that owns more than one factory able to produce the object's category multiplies the build time by `MultipleFactory` once for each factory past the first, dropping any fraction of a frame after each multiplication. It counts the house's structures whose [`Factory=`](/keys/factory/) names that category, switched on or not. [`MultipleFactoryCap`](/keys/multiplefactorycap/) limits how many of them count.

A value below `1` shortens build times: at `0.8`, two factories build in 80% of the time and three in 64%. A value of `1` changes nothing, a value above `1` lengthens build times, and `0` or below skips the adjustment.
