---
key: TargetEffectivenessCoefficientDefault
summary: The TargetEffectivenessCoefficient every object type falls back on.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "0"
---

The value is substituted whenever a type's own [`TargetEffectivenessCoefficient`](/keys/targeteffectivenesscoefficient/) is zero and the section being read omits the key. No shipped section sets a figure of its own. `[General]` is read before the object types on each pass over the rules, and the figure survives into the later rules files. One line in `rules.ini` therefore reaches every type, including those the expansion rules add. An explicit `0` in a section survives its own pass; it is replaced only when [a later rules layer](/systems/target-selection/#where-the-coefficients-come-from), such as the expansion rules or the map, contains the section without the key.

The term this coefficient scales weighs how badly a candidate could hurt the object choosing, so a positive value draws every affected type toward dangerous candidates. The exception is a candidate already shooting at it, where the term's sign is reversed. Setting the value here switches that term on for the whole rules file at once, and the stock rules already set `-200`.
