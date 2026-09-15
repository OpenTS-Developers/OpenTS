---
key: TurretAnimIsExclusive
summary: Runs a building's turret animation only while its weapon is charging, in place of its second active animation.
see_also: ["TurretAnim", "ActiveAnimTwo", "TurretChargeAnimRate", "Charges"]
when_omitted:
  kind: value
  value: "no"
---

A building normally runs its turret animation and its [`ActiveAnimTwo`](/keys/activeanimtwo/) alongside each other. Set, the two never run together. The turret animation exists only while the weapon is charging or holding its charge; `ActiveAnimTwo` is stopped for as long as it does and started again once the charge is spent. A building that is not charging shows `ActiveAnimTwo` and no turret animation at all, whether it has just been built, is being repaired, or has just lost its target.

```ini title="rules.ini"
[MYOBELISK] ; a BuildingType registered in [BuildingTypes]
Primary=MyChargeLaser ; a WeaponType with Charges=yes
TurretAnim=MYOBEL_C   ; an AnimType registered in [Animations]
TurretAnimIsExclusive=yes
```

Only a building whose primary weapon is [`Charges=yes`](/keys/charges/) ever reaches the charging or charged state, so on any other building the flag leaves the turret animation suppressed for good.
