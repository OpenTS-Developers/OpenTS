---
key: TargetStrengthCoefficient
summary: Weights a candidate target by how much of its maximum strength it still has.
see_also: ["system:target-selection"]
when_omitted:
  kind: computed
  note: Takes the value of TargetStrengthCoefficientDefault in [General], which is itself 0 when that key is absent too.
---

The coefficient multiplies the candidate's current strength as a fraction of its maximum: `1` for an undamaged candidate, falling toward `0` as it is hurt. A positive value makes the object prefer healthy candidates; a negative value is what makes it finish off wounded ones. It is read from the type of the object doing the choosing, never from the candidate's type.

```ini title="rules.ini"
[MYTANK] ; example UnitType
TargetStrengthCoefficient=-1
```

:::caution[Zero cannot be pinned on one type]
The value is read with a fallback that substitutes [`TargetStrengthCoefficientDefault`](/keys/targetstrengthcoefficientdefault/) whenever the stored figure is zero, and the rules are read again for each later layer. An explicit `0` therefore lasts only until the first later layer that contains the type's section without this key, which puts the global default back.
:::
