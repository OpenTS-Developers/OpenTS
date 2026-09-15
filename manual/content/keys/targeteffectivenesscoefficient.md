---
key: TargetEffectivenessCoefficient
summary: Weights a candidate target by how much damage that candidate's warhead does to this object's armor.
see_also: ["system:target-selection"]
when_omitted:
  kind: computed
  note: Takes the value of TargetEffectivenessCoefficientDefault in [General], which is itself 0 when that key is absent too.
---

The coefficient multiplies the [`Verses`](/keys/verses/) percentage of the weapon the candidate would choose against this object, measured against this object's own [armor class](/reference/enums/armor/): how badly the candidate could hurt the object that is choosing. A positive value draws fire toward dangerous candidates. It is read from the type of the object doing the choosing, never from the candidate's type. A candidate with no weapon, or one whose weapon has no warhead, contributes nothing to this term.

:::caution[The sign flips for a candidate that is already shooting at this object]
This is the one term whose result is negated rather than added when the candidate's current target is this object. An otherwise identical bystander therefore outscores the object's own attacker by twice the term. A positive coefficient, set to make an object prefer whatever can hurt it, therefore steers it away from the thing hurting it.
:::

:::caution[Zero cannot be pinned on one type]
The value is read with a fallback that substitutes [`TargetEffectivenessCoefficientDefault`](/keys/targeteffectivenesscoefficientdefault/) whenever the stored figure is zero, and the rules are read again for each later layer. An explicit `0` therefore lasts only until the first later layer that contains the type's section without this key, which puts the global default back.
:::
