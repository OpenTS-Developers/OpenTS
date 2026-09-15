---
key: ToProtect
summary: Whether damage to the object makes its computer house call defenders back to it.
see_also: ["system:base-attacked", ComputerBaseDefenseResponse]
when_omitted:
  kind: value
  value: "no"
---

A damaged instance of a type marked this way runs the same [base defense response](/systems/base-attacked/#calling-defenders-back) a damaged structure runs, sized from the attacker's [`ThreatPosed`](/keys/threatposed/) and answered by the house's infantry and vehicles. The setting is read only for a computer house; a human house's protected object is on its own.

```ini title="rules.ini"
[MYHARV] ; example UnitType
ToProtect=yes
```

:::caution[The call goes out only on ordinary damage]
The result of the hit decides which branch of the damage routine runs, and this one is reached only when the hit changed none of the object's damage states. A protected infantry, vehicle or aircraft stays silent on the hit that takes it below half strength, on the hit that takes it into condition red, and on the killing blow. Those are the states worth calling for help about, and none of them reaches this branch. A structure is not affected: it calls for help on every hit from an attacking object regardless of result.
:::
