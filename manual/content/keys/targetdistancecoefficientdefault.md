---
key: TargetDistanceCoefficientDefault
summary: The TargetDistanceCoefficient every object type falls back on.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "0"
---

The value is substituted whenever a type's own [`TargetDistanceCoefficient`](/keys/targetdistancecoefficient/) is zero and the section being read omits the key. No shipped section sets a figure of its own. `[General]` is read before the object types on each pass over the rules, and the figure survives into the later rules files. One line in `rules.ini` therefore reaches every type, including those the expansion rules add. An explicit `0` in a section survives its own pass; it is replaced only when [a later rules layer](/systems/target-selection/#where-the-coefficients-come-from), such as the expansion rules or the map, contains the section without the key.

A positive value rewards distance on every type at once, so a rules-wide distance penalty is written as a negative number, as the stock `-1` is.
