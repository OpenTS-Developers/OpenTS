---
key: MyEffectivenessCoefficient
summary: Weights a candidate target by how much damage this object's own warhead does to that candidate's armor.
see_also: ["system:target-selection"]
when_omitted:
  kind: computed
  note: Takes the value of MyEffectivenessCoefficientDefault in [General], which is itself 0 when that key is absent too.
---

The coefficient multiplies the [`Verses`](/keys/verses/) percentage of the weapon this object would choose against the candidate, measured against the candidate's [armor class](/reference/enums/armor/). A positive value pulls the object toward candidates it damages efficiently and a negative one pushes it away from them. It is read from the type of the object doing the choosing, never from the candidate's type.

```ini title="rules.ini"
[MYTANK] ; example UnitType
MyEffectivenessCoefficient=2
```

:::caution[Zero cannot be pinned on one type]
The value is read with a fallback that substitutes [`MyEffectivenessCoefficientDefault`](/keys/myeffectivenesscoefficientdefault/) whenever the stored figure is zero, and the rules are read again for each later layer: the language rules, the expansion rules, and the map. An explicit `0` therefore lasts only until the first later layer that contains the type's section without this key, which puts the global default back. Zero survives on a type only while the global default is zero as well.
:::
