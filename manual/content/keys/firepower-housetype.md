---
key: Firepower
summary: The multiplier a country applies to the damage its houses deal by firing a weapon.
see_also: ["system:difficulty"]
when_omitted:
  kind: value
  value: "1.0"
---

A projectile an object of this country's house fires from a weapon has the weapon's [`Damage`](/keys/damage/#scope-weapontype) multiplied by this value, so a figure above 1 hits harder. Projectiles created outside that firing path deal their own damage and are never scaled: a nuke silo's launch, either kind of EM pulse, a superweapon, a trigger action, and a bullet splitting into more. The product is worked out once, [when the house is given its difficulty slot](/systems/difficulty/#how-the-figures-are-combined), and not per shot.

A campaign game drops the country's contribution, so this value shapes skirmish and multiplayer games only; the difficulty section's own [`FirePower=`](/keys/firepower-difficulty-settings/) applies in both.
