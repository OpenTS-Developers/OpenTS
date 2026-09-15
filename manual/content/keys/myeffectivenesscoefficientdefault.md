---
key: MyEffectivenessCoefficientDefault
summary: The MyEffectivenessCoefficient every object type falls back on.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "0"
---

The value is substituted whenever a type's own [`MyEffectivenessCoefficient`](/keys/myeffectivenesscoefficient/) is zero and the section being read omits the key. An explicit `0` in a section survives its own pass. It is replaced only when a later rules layer contains the section without the key. `[General]` is read before the object types in each pass over the rules, so a value written here reaches every type in the same file.

Raising it therefore switches this term of the threat score on for every type that never sets the key itself.
