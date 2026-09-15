---
key: WeaponType
summary: The WeaponType a missile silo launches for this superweapon.
see_also: ["system:superweapons"]
when_omitted:
  kind: value
  value: none
---

The silo takes the projectile, warhead, maximum speed and projectile range from this weapon and launches them with [a hard-coded strength of 200](/systems/superweapons/#multi-missile-and-chem-missile). Nothing else reads the value. The ion cannon, the EM pulse, the firestorm, the hunter seeker and the drop pods all deliver payloads of their own. A one-time missile is built from the hard-coded weapons `MultiLauncher` and `ChemLauncher` regardless of what is set here.

:::danger[The silo does not look the weapon up by section]
A silo records the *behavior number* of the weapon that ordered the launch (0 for `MultiMissile`, 5 for `ChemMissile`, 6 for `DropPods`) and at launch reads `WeaponType=` from whichever declared section stands at that position of the `[SuperWeaponTypes]` list. The fired section's own weapon is read only when it stands at that position itself, which the shipped list arranges by declaring its seven sections in behavior order. A custom `Type=MultiMissile` entry appended after the stock seven therefore grants and charges normally, but its `WeaponType=` is never read at launch. The missile flies with the weapon of the stock section standing at position 0. Only the section standing at the behavior's own position decides what a silo fires.
:::
