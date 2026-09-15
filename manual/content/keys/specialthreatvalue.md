---
key: SpecialThreatValue
summary: A per-type figure that TargetSpecialThreatCoefficient scales into a candidate's threat score.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "0"
---

The figure is read from the candidate's type and multiplied by the [`TargetSpecialThreatCoefficient`](/keys/targetspecialthreatcoefficient/) of the object doing the choosing. It is the one term of the threat score authored on the target rather than on the shooter. Raising it makes every object with a positive coefficient prefer that type, and every object with a negative coefficient avoid it.

```ini title="rules.ini"
[MYPRIZE] ; example UnitType worth hunting
SpecialThreatValue=50
```

Unlike the five threat coefficients ([`MyEffectivenessCoefficient`](/keys/myeffectivenesscoefficient/), [`TargetEffectivenessCoefficient`](/keys/targeteffectivenesscoefficient/), [`TargetSpecialThreatCoefficient`](/keys/targetspecialthreatcoefficient/), [`TargetStrengthCoefficient`](/keys/targetstrengthcoefficient/) and [`TargetDistanceCoefficient`](/keys/targetdistancecoefficient/)), this value has no global fallback in `[General]`. A type that omits the key keeps `0` and contributes nothing to the term, whatever the coefficient is set to.
