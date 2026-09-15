---
key: FirePower
summary: The multiplier a difficulty setting applies to the damage its houses deal by firing a weapon.
see_also: ["system:difficulty"]
when_omitted:
  kind: value
  value: "1"
  note: The difficulty block is re-read from fixed defaults whenever its section is present, so a later file that contains the section without this key restores 1 rather than keeping the earlier value.
---

`[Easy]`, `[Normal]` and `[Difficult]` each set their own multiplier, and a house takes the one for [the difficulty slot it is assigned](/systems/difficulty/#from-the-setting-to-a-slot). A projectile an object of that house fires from a weapon has the weapon's [`Damage`](/keys/damage/#scope-weapontype) multiplied by it, so a figure above 1 hits harder. Projectiles the engine creates outside that path deal their own damage and are never scaled: a nuke silo's launch, either kind of EM pulse, a superweapon, a trigger action, and a bullet splitting into more. A sonic weapon, and one driven by fire particles, creates its projectile with zero damage before the multiplier is reached, so neither is scaled by it.

The product is worked out once, when the house is given its slot, alongside the country's own [`Firepower=`](/keys/firepower-housetype/) outside campaign games. In a campaign game the country's figure is dropped and this one stands alone.

```ini title="rules.ini"
[Difficult]
FirePower=1.2
```
