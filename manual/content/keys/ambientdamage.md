---
key: AmbientDamage
summary: The damage a sonic wave or a railgun beam deals to everything along its path, as opposed to the damage the shot delivers at the target.
see_also: ["IsSonic", "IsRailgun", "Damage", "system:walls-and-gates"]
when_omitted:
  kind: value
  value: "0"
---

Only two weapon kinds deliver the figure as damage. An [`IsSonic=yes`](/keys/issonic/) weapon delivers it in the wave that rolls out to the target, and an [`IsRailgun=yes`](/keys/israilgun/) weapon delivers it in the line the beam punches through the world. Both apply it through the weapon's [`Warhead=`](/keys/warhead/#scope-weapontype) at a measured distance of zero, so the warhead's falloff never thins it and only its [`Verses`](/keys/verses/) percentage against the victim's armor applies.

```ini title="rules.ini"
[MySonicGun] ; example WeaponType
Damage=0        ; the wave has no projectile damage
AmbientDamage=3 ; taken by everything the wave rolls over
IsSonic=yes
Range=6
```

The wave damages afresh on every frame it covers a cell, so an object the wave lingers over is hit many times before it passes. Wall overlay in a covered cell is worn down by the same figure, and chain reactive overlay standing there is set off. The firing object itself is skipped.

A railgun beam is a single event at the moment of firing. Every object whose center lies within [`[CombatDamage] RailgunDamageRadius`](/keys/railgundamageradius/) of the beam line takes the figure once. The object aimed at takes it too, whether or not the line actually crossed it. A structure in a crossed cell is never measured against that radius and always counts.

The figure is also added to the weapon's [`Damage=`](/keys/damage/#scope-weapontype) whenever the engine works out how hard an object hits. That average is what [decides whether an object is treated as a healer and whether it answers fire](/systems/target-selection/#retaliation). A positive ambient figure on a healing weapon can therefore lift the average back above zero and cost the object its healer behavior.

:::caution[Later occupants of a cell take a reduced figure]
A sonic wave damages the occupants of a cell one after another through a single working figure. Each object leaves that figure reduced to what its own armor let through; when the blow killed it, the figure falls to the strength it had left. Only the first object in the cell is damaged with the weapon's own figure. Every object behind it in the same cell takes whatever the object before it was reduced to. Wall overlay is unaffected, since it is worn down from the weapon's figure directly.
:::
