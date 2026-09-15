---
key: MaxDamage
summary: The ceiling one application of damage is held to once armor and distance have been taken into account.
see_also: ["MinDamage", "Verses", "AtomDamage"]
when_omitted:
  kind: value
  value: "1000"
---

The cap is the last test in the same step that scales a raw figure by the warhead's [`Verses`](/keys/verses/) entry for the target's armor and thins it with the distance from the blast. Where the thinning went too far, that step lifts the result back to [`MinDamage`](/keys/mindamage/). It applies to one object taking one blow, so a blast that reaches several objects is capped separately against each of them.

Two things pass it by. A negative figure (healing) is returned before the cap. Damage the engine marks as forced skips the whole step, so a crate's direct hit on the unit that collected it lands at its written figure even if it exceeds the cap.
