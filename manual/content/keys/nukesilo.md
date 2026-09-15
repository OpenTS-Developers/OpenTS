---
key: NukeSilo
summary: Whether the structure can serve as the launch site of a MultiMissile or ChemMissile superweapon.
see_also: [SuperWeapon, WeaponType, "system:superweapons"]
when_omitted:
  kind: value
  value: "no"
---

A missile weapon looking for somewhere to fire from scans the declared BuildingTypes for the first one with this flag that also names the weapon in [`SuperWeapon=`](/keys/superweapon/) or [`SuperWeapon2=`](/keys/superweapon2/). The scan stops at that type, so a second matching silo type is never reached, even when the house has no structure of the first. It then looks for one of the firing house's structures of the first type. The flag also selects the launch sequence the structure plays: the door opens, holds while the missile leaves, and closes again.

```ini title="rules.ini"
[NAMISL]        ; Missile Silo
SuperWeapon=MultiSpecial
SuperWeapon2=ChemicalSpecial
NukeSilo=yes
```

:::caution[Without a silo the shot is spent for nothing]
The scan is the only way an ordinary missile weapon reaches a launch. A weapon whose granting structure is not `NukeSilo=yes`, or whose house has none standing at that moment, still discharges: the charge is consumed and the timer restarts, and no missile is created. A one-time missile (one whose superweapon is removed once it fires) is the exception, being built from the map edge instead of from a silo.
:::
