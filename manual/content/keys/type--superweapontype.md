---
key: Type
scope: superweapontype
label: Superweapon behavior
see_also: ["system:superweapons"]
when_omitted:
  kind: value
  value: none
---

Seven names are recognized, in any letter case: `MultiMissile`, `EMPulse`, `Firestorm`, `IonCannon`, `HunterSeeker`, `ChemMissile` and `DropPod`. Each selects one of [the effects the engine can deliver](/systems/superweapons/#what-each-behavior-delivers). Everything else about the weapon (its delay, its cameo, its cursor, the structure that grants it) comes from the other settings in the section. Several sections may therefore name the same behavior and remain independent weapons.

A name the engine does not recognize is not rejected and does not clear the value: the assignment is skipped and whatever was already in force stands. In a section that has never named a behavior, that leaves the weapon with none, and it charges, shows a cameo and can be fired while delivering nothing.

The value is also what the [firestorm defense](/systems/superweapons/#the-firestorm-defense) needs before raising and lowering the wall does anything. The drain state machine that governs it is switched on by [`UseChargeDrain=yes`](/keys/usechargedrain/) instead.
