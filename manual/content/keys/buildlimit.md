---
key: BuildLimit
summary: The cap on how many of the type a house may build, with the sign choosing which tally it is compared against.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: "2147483647"
---

A value above zero is compared against the objects of the type the house owns at that moment, so destroying one frees its slot again. A value below zero is compared, as a magnitude, against every object of the type the house has ever produced. That tally is never decremented, so the slot is gone for good. Zero is not a cap of zero in the first sense. The comparison it produces can never pass, so the type is never buildable at all.

[Build limits](/systems/production/#build-limits) covers the two adjustments that widen the owned tally, the reason a cameo does not vanish partway through a build, and the separate tally the production queue applies.
