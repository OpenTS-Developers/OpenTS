---
key: TargetSpecialThreatCoefficientDefault
summary: The TargetSpecialThreatCoefficient every object type falls back on.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "0"
---

The value is substituted whenever a type's own [`TargetSpecialThreatCoefficient`](/keys/targetspecialthreatcoefficient/) is zero and the section being read omits the key. No shipped section sets a figure of its own. `[General]` is read before the object types on each pass over the rules, and the figure survives into the later rules files. One line in `rules.ini` therefore reaches every type, including those the expansion rules add. An explicit `0` in a section survives its own pass; it is replaced only when [a later rules layer](/systems/target-selection/#where-the-coefficients-come-from), such as the expansion rules or the map, contains the section without the key.

The term this coefficient scales weighs the [`SpecialThreatValue`](/keys/specialthreatvalue/) of the candidate's type, so a positive value sends every affected type after whatever a mod marks as valuable. That figure has no fallback of its own, so a candidate whose type names none contributes nothing here. The stock `200` therefore does nothing until some type sets a value. Setting the value here switches the term on for the whole rules file at once.
