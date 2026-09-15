---
key: SuperWeapon
summary: The superweapon a standing structure of this type grants its owner.
see_also: [SuperWeapon2, AuxBuilding, "system:superweapons"]
when_omitted:
  kind: value
  value: none
---

The value names a section listed in `[SuperWeaponTypes]`, not a behavior, so two structures may grant two sections that deliver the same effect. The key holds a single superweapon. A second grant goes through the separate [`SuperWeapon2=`](/keys/superweapon2/) key. A house is [granted the weapon](/systems/superweapons/#from-a-structure-or-a-plug) while it owns at least one active, unlimboed structure of this type, and loses it again when the last one goes. Switching the structure off suspends the weapon rather than removing it. A name the engine cannot resolve to a declared section leaves the type granting nothing.

The key is read on the type, so it works both on a structure a house builds and on a plug installed into one. A plug's grant skips the [`AuxBuilding=`](/keys/auxbuilding/) test the structure's own grant goes through.

```ini title="rules.ini"
[GAPLUG3]       ; Ion Cannon Uplink, a plug for the GDI Upgrade Center
PowersUpBuilding=GAPLUG
SuperWeapon=IonCannonSpecial
```

The same value is what a [`NukeSilo=yes`](/keys/nukesilo/) type is matched against when a missile weapon looks for somewhere to launch from.
