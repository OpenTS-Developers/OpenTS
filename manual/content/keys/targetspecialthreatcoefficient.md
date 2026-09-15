---
key: TargetSpecialThreatCoefficient
summary: Weights a candidate target by the SpecialThreatValue of that candidate's type.
see_also: ["system:target-selection"]
when_omitted:
  kind: computed
  note: Takes the value of TargetSpecialThreatCoefficientDefault in [General], which is itself 0 when that key is absent too.
---

The coefficient multiplies the candidate type's [`SpecialThreatValue`](/keys/specialthreatvalue/), which is the one figure in the threat score authored on the target rather than on the shooter. Set it positive on the types that should hunt whatever a mod marks as valuable, and negative on the types that should leave those alone. It is read from the type of the object doing the choosing.

```ini title="rules.ini"
[MYHUNTER] ; example UnitType
TargetSpecialThreatCoefficient=1

[MYPRIZE] ; example UnitType worth hunting
SpecialThreatValue=50
```

:::caution[Zero cannot be pinned on one type]
The value is read with a fallback that substitutes [`TargetSpecialThreatCoefficientDefault`](/keys/targetspecialthreatcoefficientdefault/) whenever the stored figure is zero, and the rules are read again for each later layer. An explicit `0` therefore lasts only until the first later layer that contains the type's section without this key, which puts the global default back.
:::
